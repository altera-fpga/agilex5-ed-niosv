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
#include <task.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "tse_driver.h"
#include "altera_msgdma.h"
#include "intel_avalon_tse.h"

typedef alt_msgdma_prefetcher_standard_descriptor desc_t;

#define SYS_DESC_MEM_START ((uint8_t *)SYS_DESC_MEM_BASE)
#define SYS_DESC_MEM_SIZE  8192

void msgdma_init(NetworkInterface_t *pxInterfaceTSE);
int msgdma_send_copy(uint8_t *p_data, size_t size);

#define REFILL_QUEUE_LENGTH 16

#undef ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE
#define ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE (64)  /* Power-of-2 for ring buffer API; all descriptors active in ring */
#undef ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE
#define ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE (64)  /* TX ring buffer - scaled to 64 descriptors for better throughput */

#define MAX_ATTEMPT_COUNT   10

struct NET_DEV_data {

    alt_msgdma_dev *RxMsgdma;
    alt_msgdma_dev *TxMsgdma;

    // TX descriptor list.
    desc_t *TxList;
    
    // TX ring buffer context for proper TX handling
    alt_msgdma_tx_ring_ctx tx_ring_ctx;
    
    // Buffer mapping for TX ring
    void* pxTxBufferMap[ALTERA_TSE_MSGDMA_TX_DESC_CHAIN_SIZE];

    /***************************** BEGIN RX structures ************************/

    // RX descriptor lists. Let the RX mSGDMA have a list to use while we
    // (software) process the list filled with data from the TSE.
    desc_t *RxList;

    NetworkInterface_t *pxSavedInterface;

    NetworkBufferDescriptor_t *pxRxBufferMap[ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE];
    
    // Persistent array of buffer data pointers for HAL ring buffer API
    void* pxRxBufferPtrs[ALTERA_TSE_MSGDMA_RX_DESC_CHAIN_SIZE];
    
    // Ring buffer context for proper RX handling
    alt_msgdma_rx_ring_ctx rx_ring_ctx;

    // Which list is currently owned/being used by hardware? Can be -1 if
    // hardware has no list (due to sw needing to process all lists).
    alt_8 RxListHwOwned;

    // In the ISR, which descriptor to process next.
    alt_u8 RxIsrNextDesc;

    // Since we get one packet/descriptor data at a time, we need to track
    // which packet we should process next from the current pending list.
    alt_u8 RxPendListNextDesc;

    /***************************** END RX structures **************************/

    /***************************** BEGIN statistics ***************************/
    // How many Rx packets successfully receieved.
    alt_u32 StatRxSuccessPackets;

    // How many Tx packets successfully sent.
    alt_u32 StatTxSuccessPackets;

    // Statistics in the Rx ISR.
    uint32_t StatRxIsrHit;              // How many times the Rx ISR was called.
    uint32_t StatRxIsrButNoHwOwnedList; // No HW owned list set.
    uint32_t StatRxIsrQueueFull;        // Rx Queue was full when signalling.

    // Statistics in the Tx ISR.
    uint32_t StatTxIsrHit;              // How many times the Tx ISR was called.
    uint32_t StatTxIsrMsgdmaError;      // Error in mSGDMA.
    uint32_t StatTxIsrDeallocError;     // Failed to queue dealloc of Tx buffer.

    /***************************** END statistics *****************************/
};
