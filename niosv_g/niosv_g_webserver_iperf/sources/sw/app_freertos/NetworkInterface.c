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

/* ========================= FreeRTOS includes ============================== */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "semphr.h"

/* ======================== Standard Library includes ======================== */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>

/* ========================= FreeRTOS+TCP includes ========================== */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "FreeRTOS_Stream_Buffer.h"

/* ========================= TSE+MSGDMA includes ========================== */
#include "NetworkInterface.h"
#include "tse_driver.h"
#include "msgdma_driver.h"

#define ALIGNMENT       32
#define PADDING          2

/* Software statistics */
static uint32_t ulTxPackets = 0;
static uint32_t ulTxErrors  = 0;
static uint32_t ulTxDropped = 0;
static uint32_t ulRxPackets = 0;
static uint32_t ulRxDropped = 0;
static BaseType_t xPhyLinkStatus = pdFALSE;

void vPrintEthernetStats(void)
{
    printf("--- Ethernet Software Statistics ---\n");
    printf("TX Packets : %lu\n", ulTxPackets);
    printf("TX Errors  : %lu\n", ulTxErrors);
    printf("TX Dropped : %lu\n", ulTxDropped);
    printf("RX Packets : %lu\n", ulRxPackets);
    printf("RX Dropped : %lu\n", ulRxDropped);
    printf("PHY Link   : %s\n", xPhyLinkStatus ? "UP" : "DOWN");
    printf("-----------------------------------\n");
}

BaseType_t xNetworkInterfaceInitialise(NetworkInterface_t *pxInterface)
{
    FreeRTOS_debug_printf( ("xNetworkInterfaceInitialise\n") );
    static BaseType_t xInitialised = pdFALSE;
    
    if (xInitialised == pdFALSE)
    {
        printf("Initializing MAC and PHY\n");
        if(!tse_mac_phy_init(pxInterface->pxEndPoint->xMACAddress)) {                         //Configure TSE MAC and PHY registers
            printf("PHY Init Failed\n");
            return pdFALSE;
        }
        printf("Initializing MSGDMA\n");
        msgdma_init(pxInterface);                //Initialize MSGDMA

        xInitialised = pdTRUE;
    }

    printf("Performing PHY link training...\n");
    for (int retry = 0; retry < 10; retry++)
    {
        if (tse_phy_link_up())
        {
            xPhyLinkStatus = pdTRUE;
            printf("PHY link is UP.\n");
            return pdTRUE;
        }
        else
        {
            printf("PHY link down, retrying... (%d)\n", retry + 1);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
    printf("PHY link failed to come up after retries.\n");
    return pdFALSE;
}

BaseType_t xNetworkInterfaceOutput(struct xNetworkInterface *pxNetworkInterface, NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend)
{
    FreeRTOS_debug_printf( ("Entering xNetworkInterfaceOutput\n") );
    BaseType_t xReturn = pdFALSE;

    if (!tse_phy_link_up()) {
        printf("PHY link is not up. Unable to process transmit requests.\n");
        return xReturn;
    }
    if ((pxDescriptor != NULL) && (pxDescriptor->pucEthernetBuffer != NULL))
    {
        int txResult = 0;
        if (xReleaseAfterSend == pdFALSE)
        {
            txResult = msgdma_send_copy(pxDescriptor->pucEthernetBuffer,
                                       pxDescriptor->xDataLength);

            if (txResult == 0)
            {
                ulTxPackets++;
                xReturn = pdTRUE;
            }
            else
            {
                ulTxErrors++;
                vReleaseNetworkBufferAndDescriptor(pxDescriptor);
            }
        }
        else
        {
            txResult = msgdma_send_copy(pxDescriptor->pucEthernetBuffer,
                                       pxDescriptor->xDataLength);

            vReleaseNetworkBufferAndDescriptor(pxDescriptor);
            if (txResult == 0)
            {
                ulTxPackets++;
                xReturn = pdTRUE;
            }
            else
            {
                ulTxErrors++;
            }
        }
    }
    else
    {
        if (pxDescriptor == NULL)
            printf("Cannot transmit - null descriptor\n");
        if (pxDescriptor->pucEthernetBuffer == NULL)
            printf("Cannot transmit - null buffer\n");
        printf("All dropped\n");
        ulTxDropped++;
    }

    return xReturn;
}

BaseType_t xGetPhyLinkStatus(struct xNetworkInterface * pxInterface)
{
    return tse_phy_link_up() ? pdTRUE : pdFALSE;
}

void vNetworkInterfaceAllocateRAMToBuffers(NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ])
{
    FreeRTOS_debug_printf( ("vNetworkInterfaceAllocateRAMToBuffers") );

    static uint8_t ucBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS * (ipTOTAL_ETHERNET_FRAME_SIZE + ALIGNMENT + PADDING) ] __attribute__((aligned(32)));

    for (UBaseType_t uxIndex = 0; uxIndex < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; uxIndex++)
    {
        uint8_t *pucUnaligned = &ucBuffers[uxIndex * (ipTOTAL_ETHERNET_FRAME_SIZE + ALIGNMENT + PADDING)];
        uintptr_t ulAddress = (uintptr_t)pucUnaligned;
        
        ulAddress = (ulAddress + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
        if (((ulAddress + 2) % 4) != 0)
        {
            ulAddress += 2;
        }
        pxNetworkBuffers[uxIndex].pucEthernetBuffer = (uint8_t *)ulAddress;
    }
    return;
}

NetworkInterface_t *pxFillInterfaceDescriptor( BaseType_t xEMACIndex, NetworkInterface_t *pxInterface )
{
    static const char *pcInterfaceName = "eth_tse";

    configASSERT( pxInterface != NULL );

    pxInterface->pcName                = pcInterfaceName;
    pxInterface->pvArgument            = NULL;
    pxInterface->pfInitialise          = xNetworkInterfaceInitialise;
    pxInterface->pfOutput              = xNetworkInterfaceOutput;
    pxInterface->pfGetPhyLinkStatus    = xGetPhyLinkStatus;

    FreeRTOS_AddNetworkInterface( pxInterface );

    return pxInterface;
}
