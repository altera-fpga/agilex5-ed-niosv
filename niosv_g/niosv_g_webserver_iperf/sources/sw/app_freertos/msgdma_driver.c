/*
 * License Agreement
 *
 * Copyright (c) 2025
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

#include <task.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include "sys/alt_cache.h"
#include "msgdma_driver.h"
#include "io.h"
#include "altera_msgdma_prefetcher_regs.h"

#define MSGDMA_DEBUG 0

static TaskHandle_t xRxTask = NULL;
static size_t sys_desc_mem_index = 0;

// Prefetcher standard descriptors require 32-byte alignment.
#define DESC_ALIGNMENT 32

void* sys_desc_malloc(size_t size)
{
    /* Pad current index to the next alignment boundary */
    size_t padding = (DESC_ALIGNMENT - (sys_desc_mem_index % DESC_ALIGNMENT)) % DESC_ALIGNMENT;
    size_t aligned_index = sys_desc_mem_index + padding;

    /* Round size up to alignment */
    size_t aligned_size = (size + DESC_ALIGNMENT - 1) & ~(DESC_ALIGNMENT - 1);

    if (aligned_index + aligned_size > SYS_DESC_MEM_SIZE) {
        return NULL; // Not enough memory
    }

    void* ptr = SYS_DESC_MEM_START + aligned_index;
    sys_desc_mem_index = aligned_index + aligned_size;

    // Zero out the allocated memory
    memset(ptr, 0, aligned_size);
    return ptr;
}

ALTERA_MSGDMA_CSR_PREFETCHER_CSR_INSTANCE (SYS_TSE_MSGDMA_RX, SYS_TSE_MSGDMA_RX_CSR, SYS_TSE_MSGDMA_RX_PREFETCHER_CSR, sys_tse_msgdma_rx);
ALTERA_MSGDMA_CSR_PREFETCHER_CSR_INSTANCE (SYS_TSE_MSGDMA_TX, SYS_TSE_MSGDMA_TX_CSR, SYS_TSE_MSGDMA_TX_PREFETCHER_CSR, sys_tse_msgdma_tx);

// Start an mSGDMA with a descriptor list.
//  - Park mode is disabled.
//  - Poll mode is disabled.
//  - Last descriptor should be owned by SW before started.
//  - Flush descriptor list from dcache.
#define MSGDMA_START_LIST(msgdma, list) alt_msgdma_start_prefetcher_with_std_desc_list(msgdma, list, 0, 0, 1, 1)

typedef struct NET_DEV_data NET_DEV_DATA;
NET_DEV_DATA *pdev_data;

/* Print key MAC statistics - called periodically for monitoring */
static void printMACStatsCompact(void)
{
#if MSGDMA_DEBUG
    static alt_u32 call_count = 0;
    call_count++;
    
    printf("\n[MAC_STATS #%lu] TX_ok=%u RX_ok=%u if_in_err=%u if_out_err=%u\n",
           call_count,
           TSE_MAC_REGISTERS->frames_transmitted_ok,
           TSE_MAC_REGISTERS->frames_received_ok,
           TSE_MAC_REGISTERS->if_in_errors,
           TSE_MAC_REGISTERS->if_out_errors);
    printf("[MAC_STATS] if_in_ucast=%u if_in_mcast=%u if_in_bcast=%u\n",
           TSE_MAC_REGISTERS->if_in_ucast_pkts,
           TSE_MAC_REGISTERS->if_in_multicast_pkts,
           TSE_MAC_REGISTERS->if_in_broadcast_pkts);
    printf("[MAC_STATS] if_out_ucast=%u if_out_mcast=%u if_out_bcast=%u\n\n",
           TSE_MAC_REGISTERS->if_out_ucast_pkts,
           TSE_MAC_REGISTERS->if_out_multicast_pkts,
           TSE_MAC_REGISTERS->if_out_broadcast_pkts);
#endif
}

/* Print detailed DMA and Prefetcher status */
static void printDMAStatus(const char *tag)
{
#if MSGDMA_DEBUG
    alt_msgdma_dev *rx_dma = pdev_data->RxMsgdma;
    
    /* Read CSR registers */
    alt_u32 csr_status = IORD_ALTERA_MSGDMA_CSR_STATUS(rx_dma->csr_base);
    alt_u32 csr_control = IORD_ALTERA_MSGDMA_CSR_CONTROL(rx_dma->csr_base);
    alt_u32 desc_fill = IORD_ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL(rx_dma->csr_base);
    
    /* Read Prefetcher registers */
    alt_u32 pref_control = IORD_ALT_MSGDMA_PREFETCHER_CONTROL(rx_dma->prefetcher_base);
    alt_u32 pref_status = IORD_ALT_MSGDMA_PREFETCHER_STATUS(rx_dma->prefetcher_base);
    alt_u32 pref_poll_freq = IORD_ALT_MSGDMA_PREFETCHER_DESCRIPTOR_POLLING_FREQ(rx_dma->prefetcher_base);
    alt_u32 pref_next_desc_low = IORD_ALT_MSGDMA_PREFETCHER_NEXT_DESCRIPTOR_PTR_LOW(rx_dma->prefetcher_base);
    
    printf("\n[%s] === DMA STATUS ===\n", tag);
    printf("[%s] CSR_STATUS=0x%08lx CSR_CONTROL=0x%08lx DESC_FILL=%lu\n",
           tag, csr_status, csr_control, desc_fill);
    printf("[%s]   BUSY=%lu DESC_EMPTY=%lu DESC_FULL=%lu\n", tag,
           (csr_status & ALTERA_MSGDMA_CSR_BUSY_MASK) ? 1 : 0,
           (csr_status & ALTERA_MSGDMA_CSR_DESCRIPTOR_BUFFER_EMPTY_MASK) ? 1 : 0,
           (csr_status & ALTERA_MSGDMA_CSR_DESCRIPTOR_BUFFER_FULL_MASK) ? 1 : 0);
    printf("[%s]   STOPPED_ERR=%lu STOPPED_EARLY=%lu IRQ=%lu\n", tag,
           (csr_status & ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK) ? 1 : 0,
           (csr_status & ALTERA_MSGDMA_CSR_STOPPED_ON_EARLY_TERMINATION_MASK) ? 1 : 0,
           (csr_status & ALTERA_MSGDMA_CSR_IRQ_SET_MASK) ? 1 : 0);
    
    printf("[%s] === PREFETCHER STATUS ===\n", tag);
    printf("[%s] PREF_CONTROL=0x%08lx PREF_STATUS=0x%08lx\n", tag, pref_control, pref_status);
    printf("[%s]   RUN=%lu DESC_POLL_EN=%lu GLOBAL_IRQ_EN=%lu\n", tag,
           (pref_control & 0x1) ? 1 : 0,
           (pref_control & 0x2) ? 1 : 0,
           (pref_control & 0x8) ? 1 : 0);
    printf("[%s]   POLL_FREQ=0x%02lx NEXT_DESC=0x%08lx\n", tag, pref_poll_freq, pref_next_desc_low);
    printf("[%s] ===================\n\n", tag);
#endif
}

/* Dump first few descriptors from ring buffer memory */
static void printDescriptorRing(const char *tag, int num_desc)
{
#if MSGDMA_DEBUG
    alt_msgdma_prefetcher_standard_descriptor *desc = (alt_msgdma_prefetcher_standard_descriptor *)pdev_data->RxList;
    
    printf("\n[%s] === DESCRIPTOR RING (first %d) ===\n", tag, num_desc);
    for (int i = 0; i < num_desc && i < 8; i++) {
        printf("[%s] Desc[%d] @ 0x%p:\n", tag, i, (void*)&desc[i]);
        printf("[%s]   read_addr=0x%08lx write_addr=0x%08lx\n", tag,
               desc[i].read_address, desc[i].write_address);
        printf("[%s]   length=%lu control=0x%08lx\n", tag,
               desc[i].transfer_length, desc[i].control);
        printf("[%s]   OWN_BY_HW=%lu END_ON_EOP=%lu XFER_IRQ=%lu\n", tag,
               (desc[i].control & (1<<31)) ? 1 : 0,
               (desc[i].control & (1<<13)) ? 1 : 0,
               (desc[i].control & (1<<14)) ? 1 : 0);
        printf("[%s]   next_desc=0x%08lx bytes_xfer=%lu status=0x%04x\n", tag,
               desc[i].next_desc_ptr, desc[i].bytes_transfered, desc[i].status);
    }
    printf("[%s] ================================\n\n", tag);
#endif
}

/* Diagnostic task removed - kept code focused on packet processing */
static void vDiagnosticPollingTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static BaseType_t msgdma_InitRxDescList(NET_DEV_DATA *pdev_data, bool refill)
{
    alt_u8     desc_idx;
    alt_u8    *write_address = NULL;
    alt_u32        control;
    alt_u32        net_buf_size;
    desc_t        *desc;
    int            result;
    NetworkBufferDescriptor_t *pxBuffer = NULL;

    net_buf_size = ALTERA_TSE_MAC_MAX_FRAME_LENGTH;
    configASSERT(ALTERA_TSE_MAC_MAX_FRAME_LENGTH <= pdev_data->RxMsgdma->max_byte);

    /* Use ring buffer API for initialization */
    if (refill == pdFALSE) {
        /* Allocate buffers for ALL descriptors in the ring */
        for (desc_idx = 0; desc_idx < ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE; desc_idx++) {
            pxBuffer = pxGetNetworkBufferWithDescriptor(ALTERA_TSE_MAC_MAX_FRAME_LENGTH, 1000);
            if (pxBuffer == NULL) {
                printf("[ERR] Failed to allocate buffer for descriptor %d\n", desc_idx);
                return pdFAIL;
            }
            pdev_data->pxRxBufferMap[desc_idx] = pxBuffer;
            pdev_data->pxRxBufferPtrs[desc_idx] = pxBuffer->pucEthernetBuffer;  /* Store in persistent array */
        }
        
        /* Initialize ring buffer using API with persistent buffer pointer array */
        result = alt_msgdma_rx_ring_init(
            pdev_data->RxMsgdma,
            &pdev_data->rx_ring_ctx,
            pdev_data->RxList,
            ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE,
            pdev_data->pxRxBufferPtrs,  /* Use persistent array, not stack-local */
            ALTERA_TSE_MAC_MAX_FRAME_LENGTH
        );
        
        if (result != 0) {
            printf("[ERR] Ring buffer initialization failed: %d\n", result);
            return pdFAIL;
        }
        
        return pdPASS;
    }
    
    /* Refill path now handled directly in refillDescriptor() */
    return pdPASS;
}

static  BaseType_t  msgdma_InitTxDescList (NET_DEV_DATA *pdev_data)
{
    int result;
    alt_u32 desc_idx;
    
#if MSGDMA_DEBUG
    printf("[TX_INIT] ========================================\n");
    printf("[TX_INIT] Initializing TX ring with %d descriptors\n", 
           ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE);
    printf("[TX_INIT] TxList base address: %p\n", pdev_data->TxList);
#endif
    
    /* Initialize TX ring buffer using HAL API */
    result = alt_msgdma_tx_ring_init(
        pdev_data->TxMsgdma,
        &pdev_data->tx_ring_ctx,
        pdev_data->TxList,
        ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE
    );
    
    if (result != 0) {
        printf("[TX_INIT] ERROR: Ring initialization failed: %d\n", result);
        return pdFAIL;
    }
    
    /* Assign buffer map to ring context */
    pdev_data->tx_ring_ctx.buffer_map = pdev_data->pxTxBufferMap;
    
#if MSGDMA_DEBUG
    /* DEBUG: Print ring structure to verify circular linking */
    printf("[TX_INIT] Ring structure verification:\n");
    for (desc_idx = 0; desc_idx < ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE; desc_idx++) {
        desc_t *desc = &pdev_data->TxList[desc_idx];
        printf("[TX_INIT]   Desc[%d]: addr=%p next_ptr=0x%08lx control=0x%08lx\n",
               desc_idx, desc, (unsigned long)desc->next_desc_ptr, 
               (unsigned long)desc->control);
    }
    
    /* Verify circular linking: last descriptor should point to first */
    desc_t *last_desc = &pdev_data->TxList[ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE - 1];
    desc_t *first_desc = &pdev_data->TxList[0];
    alt_u32 expected_next = (alt_u32)((alt_u64)first_desc & 0xFFFFFFFF);
    
    if (last_desc->next_desc_ptr == expected_next) {
        printf("[TX_INIT] ✓ Circular link verified: Desc[%d]->next points to Desc[0]\n",
               ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE - 1);
    } else {
        printf("[TX_INIT] ✗ WARNING: Circular link broken! Expected 0x%08lx, got 0x%08lx\n",
               (unsigned long)expected_next, (unsigned long)last_desc->next_desc_ptr);
    }
    
    printf("[TX_INIT] Ring context: tx_prod=%lu tx_cons=%lu ring_size=%lu\n",
           (unsigned long)pdev_data->tx_ring_ctx.tx_prod,
           (unsigned long)pdev_data->tx_ring_ctx.tx_cons,
           (unsigned long)pdev_data->tx_ring_ctx.ring_size);
    
    printf("[TX_INIT] Success - TX Ring initialized\n");
    printf("[TX_INIT] ========================================\n");
#endif
    
    return pdPASS;
}

static  BaseType_t  msgdma_InitRxAndTxDescLists (NET_DEV_DATA *dev)
{
    // Initialize RX descriptor List.
    {
        FreeRTOS_debug_printf( ("RX mSGDMA Descriptor Initialization: Starting.\n") );

        if (!msgdma_InitRxDescList(dev, pdFALSE)) {
            printf("RX mSGDMA Descriptor Initialization: FAILED.\n");
            return pdFAIL;
        }

        printf("RX mSGDMA Descriptor Initialization: Success.\n");
    }
    // Initialize Tx descriptor list.
    {
        FreeRTOS_debug_printf( ("Tx descriptor list initialization: Starting.\n") );

        if (!msgdma_InitTxDescList(dev)) {
            printf("Tx descriptor list initialization: FAILED.\n");
            return pdFAIL;
        }

        printf("Tx descriptor list initialization: Success.\n");
    }
    return pdPASS;
}

void msgdma_tx_isr(NET_DEV_DATA *pdev_data)
{
    alt_u32       reg_data;
    desc_t       *desc;

    pdev_data->StatTxIsrHit += 1;

    printf("[TX_ISR] *** ENTRY ***: hit=%lu\n", (unsigned long)pdev_data->StatTxIsrHit);
    reg_data = IORD_ALTERA_MSGDMA_CSR_STATUS(pdev_data->TxMsgdma->csr_base);
    printf("[TX_ISR] CSR status=0x%08lx (BUSY=%d ERROR=%d)\n", 
           (unsigned long)reg_data,
           (int)((reg_data & ALTERA_MSGDMA_CSR_BUSY_MASK) != 0),
           (int)((reg_data & ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK) != 0));

    /* Check for actual errors */
    if ((reg_data & ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK) ||
        (reg_data & ALTERA_MSGDMA_CSR_STOPPED_ON_EARLY_TERMINATION_MASK)) {
        pdev_data->StatTxIsrMsgdmaError += 1;
        printf("[TX_ISR] ERROR: DMA stopped on error\n");
        return;
    }

    /* Only process if DMA is NOT busy (i.e., descriptor completed) */
    if (reg_data & ALTERA_MSGDMA_CSR_BUSY_MASK) {
        printf("[TX_ISR] BUSY - DMA still running, ignoring spurious interrupt\n");
        /* Write CSR to clear the interrupt flag without processing */
        IOWR_ALTERA_MSGDMA_CSR_STATUS(pdev_data->TxMsgdma->csr_base, reg_data);
        printf("[TX_ISR] *** EXIT *** (spurious, DMA not done)\n");
        return;
    }

    /* DMA is idle/completed - process the completion */
    printf("[TX_ISR] DMA IDLE - descriptor completed!\n");
    IOWR_ALTERA_MSGDMA_CSR_STATUS(pdev_data->TxMsgdma->csr_base, reg_data);
    
    pdev_data->StatTxSuccessPackets += 1;
    printf("[TX_ISR] TX success count: %lu\n", (unsigned long)pdev_data->StatTxSuccessPackets);

    /* Cleanup data buffer */
    desc = &(pdev_data->TxList[0]);
    desc->read_address = 0;
    printf("[TX_ISR] *** EXIT *** - packet sent successfully\n");

    return;
}

/* ===== OLD RING BUFFER APPROACH (commented out - causes dispatcher errors) =====
 * The dynamic refill model corrupts the descriptor chain while hardware is processing.
 * Keeping this for reference - will use simpler sw_iperf-style sequential polling instead.

void refillDescriptor_OLD_RING_BUFFER(uint8_t index)
{
    ... (old code omitted for clarity)
}
===== END OLD RING BUFFER CODE =====  */

void refillDescriptor(uint8_t index)
{
    /* Simple sw_iperf-style approach: sequentially poll all 8 pre-allocated buffers
     * without dynamic refill. Descriptors are pre-linked in a circle with fixed buffers.
     */
    alt_msgdma_prefetcher_standard_descriptor *desc;
    int i, pkt_len;
    NetworkBufferDescriptor_t *pxBuffer;
    static uint32_t next_desc_to_process = 0;  /* Track which descriptor we're checking */
    
    /* Print MAC statistics periodically for monitoring */
    printMACStatsCompact();
    
    /* === HARDWARE DIAGNOSTIC: Check prefetcher state === */
    alt_u32 csr_status = IORD_ALTERA_MSGDMA_CSR_STATUS(pdev_data->RxMsgdma->csr_base);
  
    /* === ERROR RECOVERY: Clear error state if detected === */
    if (csr_status & ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK) {
#if MSGDMA_DEBUG 
        printf("[RX_ERROR_RECOVERY] STOPPED_ON_ERROR detected! Clearing error bit and restarting prefetcher\n");
#endif        
        /* Write-1-to-clear: Write 0x80 to clear bit 7 (STOPPED_ON_ERROR) */
        IOWR_ALTERA_MSGDMA_CSR_STATUS(pdev_data->RxMsgdma->csr_base, ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK);
        /* Read back to verify error cleared */
        csr_status = IORD_ALTERA_MSGDMA_CSR_STATUS(pdev_data->RxMsgdma->csr_base);
#if MSGDMA_DEBUG 
        printf("[RX_ERROR_RECOVERY] After clear: CSR_STATUS=0x%08lx STOPPED_ERR=%lu\n",
               csr_status, (csr_status & ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK) ? 1 : 0);
#endif        
        /* If error persists, force reset the prefetcher */
        if (csr_status & ALTERA_MSGDMA_CSR_STOPPED_ON_ERROR_MASK) {
#if MSGDMA_DEBUG 
            printf("[RX_ERROR_RECOVERY] Error persists! Forcing CSR software reset\n");
#endif
	IOWR_ALTERA_MSGDMA_CSR_CONTROL(pdev_data->RxMsgdma->csr_base, ALTERA_MSGDMA_CSR_RESET_MASK);
            /* Wait for reset to complete */
            usleep(1000);
            /* Re-read status after reset */
            csr_status = IORD_ALTERA_MSGDMA_CSR_STATUS(pdev_data->RxMsgdma->csr_base);
#if MSGDMA_DEBUG 
            printf("[RX_ERROR_RECOVERY] After reset: CSR_STATUS=0x%08lx\n", csr_status);
            
            /* === Restore prefetcher state after reset === */
            printf("[RX_ERROR_RECOVERY] Restoring prefetcher to continuous polling mode\n");
#endif            
            /* Restore descriptor pointer (points to first descriptor in ring) */
            alt_u32 ring_phys = (alt_u32)((alt_u64)pdev_data->RxList & 0xFFFFFFFF);
            IOWR_ALT_MSGDMA_PREFETCHER_NEXT_DESCRIPTOR_PTR_LOW(pdev_data->RxMsgdma->prefetcher_base, ring_phys);
            
            /* Set polling frequency */
            IOWR_ALT_MSGDMA_PREFETCHER_DESCRIPTOR_POLLING_FREQ(pdev_data->RxMsgdma->prefetcher_base, 0xFF);
            
            /* Restore control: RUN(bit0=0x01) + DESC_POLL_EN(bit1=0x02) + INTR_EN(bit3=0x08) */
            alt_u32 ctrl = 0x0b;
            IOWR_ALT_MSGDMA_PREFETCHER_CONTROL(pdev_data->RxMsgdma->prefetcher_base, ctrl);
            
            /* Read back to verify hardware actually set the register */
            alt_u32 ctrl_readback = IORD_ALT_MSGDMA_PREFETCHER_CONTROL(pdev_data->RxMsgdma->prefetcher_base);
#if MSGDMA_DEBUG 
            printf("[RX_ERROR_RECOVERY] Prefetcher restored: control=0x%08lx (RUN=%d POLL=%d INTR=%d)\n",
                   ctrl_readback,
                   (int)(ctrl_readback & 0x1),
                   (int)((ctrl_readback >> 1) & 0x1),  /* DESC_POLL_EN is bit 1 */
                   (int)((ctrl_readback >> 3) & 0x1));
#endif 
        }
    }
    
    /* Scan all 8 descriptors starting from where we left off */
    for (i = 0; i < ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE; i++) {
        int desc_idx = (next_desc_to_process + i) % ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE;
        desc = &pdev_data->RxList[desc_idx];
        
        /* Invalidate cache BEFORE reading to see latest hardware updates.
         * This ensures we see the descriptor state that HW wrote, not stale cached values.
         */
        alt_dcache_flush_no_writeback(desc, sizeof(*desc));
        //__asm__ __volatile__("" : : : "memory");  /* Compiler barrier */
	atomic_thread_fence(memory_order_seq_cst);
        
        /* Check if this descriptor is complete (bit30 cleared by hardware) */
        if (!(desc->control & (1 << 30))) {
            /* Hardware released this descriptor - packet is ready! */
            pkt_len = desc->bytes_transfered & 0xFFFF;
#if MSGDMA_DEBUG
            printf("[RX_REFILL] Packet received on desc[%d]: len=%d\n", desc_idx, pkt_len);
#endif        
            /* Get the buffer for this descriptor */
            pxBuffer = pdev_data->pxRxBufferMap[desc_idx];
            if (pxBuffer == NULL) {
                printf("[RX_REFILL] ERROR: No buffer mapped for desc[%d]\n", desc_idx);
                continue;
            }
            
            /* Invalidate buffer cache to see packet data written by DMA */
            alt_dcache_flush_no_writeback(pxBuffer->pucEthernetBuffer, pkt_len);
            //__asm__ __volatile__("" : : : "memory");  /* Compiler barrier */
		atomic_thread_fence(memory_order_seq_cst);
            pxBuffer->xDataLength = pkt_len;
            
#if MSGDMA_DEBUG
            printf("Rx Desc write address = 0x%x length = %d\n", desc->write_address, pkt_len);
            for (int j = 0; j < pkt_len && j < 64; j++) {  /* Limit to first 64 bytes */
                printf("%02X ", pxBuffer->pucEthernetBuffer[j]);
            }
            printf("\n");
#endif
            
            IPStackEvent_t xRxEvent = {
                .eEventType = eNetworkRxEvent,
                .pvData = pxBuffer
            };
            pxBuffer->pxInterface = pdev_data->pxSavedInterface;
            pxBuffer->pxEndPoint = FreeRTOS_MatchingEndpoint(pdev_data->pxSavedInterface, pxBuffer->pucEthernetBuffer);
            
            if (xSendEventStructToIPTask(&xRxEvent, 0) != pdFALSE) {
                pdev_data->StatRxSuccessPackets += 1;
#if MSGDMA_DEBUG
                printf("[RX_REFILL] Packet %lu sent to IP stack (desc[%d])\n", 
                       (unsigned long)pdev_data->StatRxSuccessPackets, desc_idx);
#endif
                
                /* Re-arm descriptor for next packet using same control bits as initialization */
                desc->bytes_transfered = 0;
                desc->status = 0;
                desc->control =
                    ALT_MSGDMA_PREFETCHER_DESCRIPTOR_CTRL_OWN_BY_HW_SET_MASK |
                    ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GO_MASK |
                    ALTERA_MSGDMA_DESCRIPTOR_CONTROL_END_ON_EOP_MASK |
                    ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK |
                    ALTERA_MSGDMA_DESCRIPTOR_CONTROL_EARLY_TERMINATION_IRQ_MASK;
                
                /* Flush descriptor to memory so DMA hardware sees the updated control bits */
                alt_dcache_flush(desc, sizeof(*desc));
        //        __asm__ __volatile__("" : : : "memory");  /* Compiler barrier - ensure DMA sees write */
	atomic_thread_fence(memory_order_seq_cst);
                
#if MSGDMA_DEBUG
                printf("[RX_REFILL] Re-armed desc[%d] with control=0x%08lx\n", desc_idx, desc->control);
#endif
            } else {
                printf("[RX_REFILL] ERROR: Failed to queue packet to IP stack\n");
                /* Don't re-arm on error - let it remain completed */
            }
        }
    }
    
    /* Update next descriptor to check on next poll */
    next_desc_to_process = (next_desc_to_process + i) % ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE;
}

static void vRefillRxBufferTask(void *pvParameters)
{
    xRxTask = xTaskGetCurrentTaskHandle();

    for (;;)
    {
        /* Wait for ISR with timeout, but also poll continuously to catch packets
         * that don't generate interrupts (fallback polling) */
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));  // 100ms timeout instead of FOREVER
        
        // printf("[RX_TASK_POLL] Woken (ISR or timeout) - checking for packets\n");
        refillDescriptor(0);
    }
}

void msgdma_rx_isr(NET_DEV_DATA *pdev_data)
{
    BaseType_t xRefillTaskWoken = pdFALSE;

    pdev_data->StatRxIsrHit += 1;
#if MSGDMA_DEBUG
    printf("[RX_ISR] *** FIRED *** hit=%lu - waking refill task\n", (unsigned long)pdev_data->StatRxIsrHit);
#endif

    /* Wake refill task to process ALL available packets in batch */
    vTaskNotifyGiveFromISR(xRxTask, &xRefillTaskWoken);
    portYIELD_FROM_ISR(xRefillTaskWoken);

    /* Acknowledge/clear prefetcher IRQ (write-1-to-clear) to allow subsequent interrupts */
    if (pdev_data && pdev_data->RxMsgdma && pdev_data->RxMsgdma->prefetcher_base) {
        IOWR_ALT_MSGDMA_PREFETCHER_STATUS(pdev_data->RxMsgdma->prefetcher_base,
                                          ALT_MSGDMA_PREFETCHER_STATUS_IRQ_SET_MASK);
#if MSGDMA_DEBUG
        printf("[RX_ISR] Cleared prefetcher IRQ status\n");
#endif
    }
}

void msgdma_init(NetworkInterface_t *pxInterface)
{
    pdev_data = (NET_DEV_DATA *)malloc(sizeof(NET_DEV_DATA));

    if (pdev_data == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    pdev_data->TxMsgdma = NULL;
    pdev_data->RxMsgdma = NULL;

    pdev_data->pxSavedInterface = pxInterface;
    // Set RX state trackers.
    pdev_data->RxListHwOwned = -1;
    pdev_data->RxIsrNextDesc = 0;
    pdev_data->RxPendListNextDesc = 0;

    // Zero statistic counters.
    pdev_data->StatRxSuccessPackets = 0;
    pdev_data->StatTxSuccessPackets = 0;

    pdev_data->StatRxIsrHit = 0;
    pdev_data->StatRxIsrButNoHwOwnedList = 0;
    pdev_data->StatRxIsrQueueFull = 0;

    pdev_data->StatTxIsrHit = 0;
    pdev_data->StatTxIsrMsgdmaError = 0;
    pdev_data->StatTxIsrDeallocError = 0;

    // Initialize mSGDMA memory.
    // RX ring buffer API needs 1X descriptors (HAL manages the ring internally)
    // TX just needs standard descriptor list
    size_t rx_list_size = ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE * sizeof(desc_t);
    size_t tx_list_size = ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE * sizeof(desc_t);

    // Allocate descriptor lists on the heap.
    {
        pdev_data->RxList = (desc_t *) sys_desc_malloc(rx_list_size);
        if (pdev_data->RxList == NULL) {
            printf("RX descriptor allocation failed\n");
            return;
        }

        pdev_data->TxList = (desc_t *) sys_desc_malloc(tx_list_size);
        if (pdev_data->TxList == NULL) {
            printf("TX descriptor allocation failed\n");
            return;
        }
    }

    //printf("Acquiring RX and TX alt_msgdma_dev: Starting.\n");
    pdev_data->RxMsgdma = alt_msgdma_open(sys_tse_msgdma_rx.name);
    if (pdev_data->RxMsgdma == NULL) {
        printf("Rx mSGDMA Open '%s': FAILED.\n", sys_tse_msgdma_rx.name);
        return;
    }

    pdev_data->TxMsgdma = alt_msgdma_open(sys_tse_msgdma_tx.name);
    if (pdev_data->TxMsgdma == NULL) {
        printf("TX mSGDMA open failed\n");
        return;
    }

    //Register mSGDMA descriptor ISR.
    {
        alt_msgdma_register_callback(
                pdev_data->RxMsgdma,                        // mSGDMA device
                (alt_msgdma_callback) msgdma_rx_isr, // callback function
                0,                                          // control
                pdev_data);                                       // arguments/context

        /* DISABLED: TX ISR 
         * TX completion is not critical - packets are sent synchronously
        alt_msgdma_register_callback(
                pdev_data->TxMsgdma,                        // mSGDMA device
                (alt_msgdma_callback) msgdma_tx_isr, // callback function
                0,                                          // control
                NULL);                                       // arguments/context
        */
    }
    //Initialize mSGDMA descriptors (and descriptor lists).
    {
        FreeRTOS_debug_printf( ("mSGDMA descriptor list initialization: Starting.\n") );
        if (!msgdma_InitRxAndTxDescLists(pdev_data)) {
            printf("mSGDMA descriptor list initialization: FAILED.\n");
            return;
        }
        
        /* Start RX ring prefetcher */
        pdev_data->RxListHwOwned = 0;
        alt_u32 result = alt_msgdma_rx_ring_start(pdev_data->RxMsgdma, &pdev_data->rx_ring_ctx);
        if (result != 0) {
            printf("[ERR] RX mSGDMA ring start failed: %d\n", result);
            return;
        }
        
#if MSGDMA_DEBUG
        /* ===== PHASE 3: Start TX ring prefetcher ===== */
        printf("[TX_PHASE3] ========================================\n");
        printf("[TX_PHASE3] Starting TX prefetcher\n");
        printf("[TX_PHASE3] TxMsgdma device: %p\n", pdev_data->TxMsgdma);
        printf("[TX_PHASE3] TX ring context: %p\n", &pdev_data->tx_ring_ctx);
        printf("[TX_PHASE3] Ring state before start: tx_prod=%lu tx_cons=%lu\n",
               (unsigned long)pdev_data->tx_ring_ctx.tx_prod,
               (unsigned long)pdev_data->tx_ring_ctx.tx_cons);
#endif        
        result = alt_msgdma_tx_ring_start(pdev_data->TxMsgdma, &pdev_data->tx_ring_ctx);
        if (result != 0) {
            printf("[TX_PHASE3] ERROR: TX ring start failed: %d\n", result);
            printf("[TX_PHASE3] ========================================\n");
            return;
        }
        
        /* Verify TX prefetcher registers */
        alt_u32 tx_pref_control = IORD_ALT_MSGDMA_PREFETCHER_CONTROL(pdev_data->TxMsgdma->prefetcher_base);
        alt_u32 tx_pref_status = IORD_ALT_MSGDMA_PREFETCHER_STATUS(pdev_data->TxMsgdma->prefetcher_base);
        alt_u32 tx_poll_freq = IORD_ALT_MSGDMA_PREFETCHER_DESCRIPTOR_POLLING_FREQ(pdev_data->TxMsgdma->prefetcher_base);
        alt_u32 tx_next_desc = IORD_ALT_MSGDMA_PREFETCHER_NEXT_DESCRIPTOR_PTR_LOW(pdev_data->TxMsgdma->prefetcher_base);
#if MSGDMA_DEBUG       
        printf("[TX_PHASE3] VERIFY: control=0x%08lx status=0x%08lx poll_freq=0x%02lx next_desc=0x%08lx\n",
               tx_pref_control, tx_pref_status, tx_poll_freq, tx_next_desc);
        printf("[TX_PHASE3] VERIFY: RUN=%d POLL_EN=%d INTR_EN=%d\n",
               (int)(tx_pref_control & 0x1),
               (int)((tx_pref_control >> 1) & 0x1),  /* Bit 1, not bit 2! */
               (int)((tx_pref_control >> 3) & 0x1));
        
        printf("[TX_PHASE3] TX prefetcher started successfully\n");
        printf("[TX_PHASE3] ========================================\n");
#endif         
        /* Enable MAC TX/RX AFTER DMA is started (matches Linux tse_set_mac) */
        alt_u32 cmd = TSE_MAC_REGISTERS->command_config;
        cmd |= MAC_CMDCFG_TX_ENA | MAC_CMDCFG_RX_ENA;
        TSE_MAC_REGISTERS->command_config = cmd;
        printf("[MSGDMA_INIT] MAC TX and RX enabled\n");
    }
    
    BaseType_t task_result = xTaskCreate(vRefillRxBufferTask, "RxRefill", 2048, NULL, tskIDLE_PRIORITY + 1, &xRxTask);
    if (task_result != pdPASS) {
        printf("Failed to create Refill task!\n");
    }
    
    /* Diagnostic task removed - kept code focused on packet processing */
	return;
}

int msgdma_send_copy(uint8_t *p_data, size_t size)
{
    int rc;
    alt_u32 reclaimed;
    static alt_u32 send_count = 0;
    static alt_u32 total_reclaimed = 0;
    static alt_u32 ring_full_count = 0;
    alt_u32 ring_depth;
    
    send_count++;
    
    /* Calculate current ring depth (how many descriptors in use) */
    ring_depth = (pdev_data->tx_ring_ctx.tx_prod >= pdev_data->tx_ring_ctx.tx_cons) ?
                 (pdev_data->tx_ring_ctx.tx_prod - pdev_data->tx_ring_ctx.tx_cons) :
                 (pdev_data->tx_ring_ctx.ring_size - pdev_data->tx_ring_ctx.tx_cons + pdev_data->tx_ring_ctx.tx_prod);
    
#if MSGDMA_DEBUG
    /* Print batching statistics every 10 sends (for ping testing) */
    if (send_count % 10 == 0) {
        printf("[TX_BATCH_STATS #%lu] ring_depth=%lu/%lu reclaimed_total=%lu ring_full_hits=%lu prod=%lu cons=%lu\n",
               (unsigned long)send_count,
               (unsigned long)ring_depth,
               (unsigned long)pdev_data->tx_ring_ctx.ring_size,
               (unsigned long)total_reclaimed,
               (unsigned long)ring_full_count,
               (unsigned long)pdev_data->tx_ring_ctx.tx_prod,
               (unsigned long)pdev_data->tx_ring_ctx.tx_cons);
    }
#endif
    
#if MSGDMA_DEBUG
    printf("[TX_SEND_PHASE2] ========================================\n");
    printf("[TX_SEND_PHASE2] Entry: buffer=%p size=%lu\n", p_data, (unsigned long)size);
    printf("[TX_SEND_PHASE2] Ring state BEFORE: tx_prod=%lu tx_cons=%lu\n",
           (unsigned long)pdev_data->tx_ring_ctx.tx_prod,
           (unsigned long)pdev_data->tx_ring_ctx.tx_cons);
#endif
    
    /* Flush data buffer cache before DMA reads it */
    alt_dcache_flush(p_data, size);
    //__asm__ __volatile__("" : : : "memory");
	atomic_thread_fence(memory_order_seq_cst);
    
    /* Proactively reclaim completed TX buffers to keep ring flowing smoothly */
    reclaimed = alt_msgdma_tx_ring_reclaim(&pdev_data->tx_ring_ctx, NULL, NULL);
    total_reclaimed += reclaimed;
    
    /* Queue packet to TX ring buffer */
    rc = alt_msgdma_tx_ring_send(
        pdev_data->TxMsgdma,
        &pdev_data->tx_ring_ctx,
        p_data,
        size
    );
    
    if (rc != 0) {
        printf("[TX_SEND_PHASE2] ERROR: Ring send failed with code %d\n", rc);
        if (rc == -ENOSPC) {
            ring_full_count++;
            printf("[TX_SEND_PHASE2] Ring FULL! depth=%lu attempting reclaim\n", (unsigned long)ring_depth);
            
            /* Try to reclaim completed buffers */
            reclaimed = alt_msgdma_tx_ring_reclaim(&pdev_data->tx_ring_ctx, NULL, NULL);
            total_reclaimed += reclaimed;
            printf("[TX_SEND_PHASE2] Reclaimed %lu buffers\n", (unsigned long)reclaimed);
            
            /* Retry send after reclaim */
            if (reclaimed > 0) {
                rc = alt_msgdma_tx_ring_send(pdev_data->TxMsgdma, &pdev_data->tx_ring_ctx, p_data, size);
#if MSGDMA_DEBUG
                if (rc == 0) {
                    printf("[TX_SEND_PHASE2] Retry SUCCESS after reclaim\n");
                } else {
                    printf("[TX_SEND_PHASE2] Retry FAILED: %d\n", rc);
                }
#endif
            }
        }
        
        if (rc != 0) {
            return -1;
        }
    }
    
#if MSGDMA_DEBUG
    printf("[TX_SEND_PHASE2] Ring state AFTER: tx_prod=%lu tx_cons=%lu\n",
           (unsigned long)pdev_data->tx_ring_ctx.tx_prod,
           (unsigned long)pdev_data->tx_ring_ctx.tx_cons);
    printf("[TX_SEND_PHASE2] Packet queued successfully\n");
    printf("[TX_SEND_PHASE2] ========================================\n");
#endif
    
    return 0;
}

void msgdma_print_statistics()
{
    uint32_t    rx_success;
    uint32_t    tx_success;

    uint32_t    rx_isr_hit;
    uint32_t    rx_isr_no_hw_owned;
    uint32_t    rx_isr_queue_full;

    uint32_t    tx_isr_hit;
    uint32_t    tx_isr_msgdma_err;
    uint32_t    tx_isr_dealloc_err;


    rx_isr_hit         = IORD_32DIRECT(&(pdev_data->StatRxIsrHit),              0);
    rx_isr_no_hw_owned = IORD_32DIRECT(&(pdev_data->StatRxIsrButNoHwOwnedList), 0);
    rx_isr_queue_full  = IORD_32DIRECT(&(pdev_data->StatRxIsrQueueFull),        0);

    tx_isr_hit         = IORD_32DIRECT(&(pdev_data->StatTxIsrHit),          0);
    tx_isr_msgdma_err  = IORD_32DIRECT(&(pdev_data->StatTxIsrMsgdmaError),  0);
    tx_isr_dealloc_err = IORD_32DIRECT(&(pdev_data->StatTxIsrDeallocError), 0);

    rx_success = IORD_32DIRECT(&(pdev_data->StatRxSuccessPackets), 0);
    tx_success = IORD_32DIRECT(&(pdev_data->StatTxSuccessPackets), 0);

    printf("Rx recvd  : %lu\n", rx_success);
    printf("Tx sent   : %lu\n", tx_success);
    printf("\n");
    printf("Rx ISR hit         : %lu\n", rx_isr_hit);
    printf("Rx ISR no hw list  : %lu\n", rx_isr_no_hw_owned);
    printf("Rx ISR queue full  : %lu\n", rx_isr_queue_full);
    printf("\n");
    printf("Tx ISR hit         : %lu\n", tx_isr_hit);
    printf("Tx ISR msgdma err  : %lu\n", tx_isr_msgdma_err);
    printf("Tx ISR dealloc err : %lu\n", tx_isr_dealloc_err);
}
