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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <io.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/termios.h>
#include "sys/ioctl.h"
#include "sys/alt_log_printf.h"
#include "system.h"
#include "altera_avalon_sysid_qsys.h"
#include "altera_avalon_sysid_qsys_regs.h"
#include "altera_avalon_jtag_uart_regs.h"
#include "altera_avalon_jtag_uart.h"
 
#include "task.h" 
#include "timers.h" 
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_TCP_IP.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_Routing.h"


#include "tse_driver.h"
#include <inttypes.h>

#define IPERF_PORT             5001
#define IPERF_STACK_SIZE       (configMINIMAL_STACK_SIZE * 4)
#define IPERF_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
#define IPERF_BUFFER_SIZE      1460   /* MTU-sized payload */
#define IPERF_REPORT_INTERVAL  1000   /* Report every 1 sec */

#define PING_TASK_STACKSIZE 2048
#define PING_TASK_PRIORITY  (configMAX_PRIORITIES - 1)

#define PING_TASK_PORTING_STATUS                	0
#define IPERF_TASK_PORTING_STATUS			1
#define APP_DEBUG					0



#if PING_TASK_PORTING_STATUS
static TaskHandle_t xPingTaskHandle = NULL;
#endif

#if IPERF_TASK_PORTING_STATUS
static TaskHandle_t xIperfTaskHandle = NULL;
#endif

#if IPERF_TASK_PORTING_STATUS
/* Runs a simple iperf-compatible TCP server for receive/send throughput tests. */
static void vIperfServerTask(void *pvParameters)
{
    Socket_t xListenSocket, xClientSocket;
    struct freertos_sockaddr xBindAddress, xClientAddress;
    socklen_t xSize = sizeof(xClientAddress);
    char *pcBuffer;
    char cMode;
    BaseType_t xBytes;
    TickType_t xLastReport;
    uint32_t ulTotalBytes = 0, ulLastReportBytes = 0;
    TickType_t xTimeoutTime = ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME;

    (void) pvParameters;

    pcBuffer = (char *) pvPortMalloc(IPERF_BUFFER_SIZE);
    if (pcBuffer == NULL) {
        printf("iperf: malloc failed\r\n");
        vTaskDelete(NULL);
    }
    memset(pcBuffer, 'A', IPERF_BUFFER_SIZE);

    xIperfTaskHandle = xTaskGetCurrentTaskHandle();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    /* Create TCP socket */
    xListenSocket = FreeRTOS_socket(FREERTOS_AF_INET,
                                    FREERTOS_SOCK_STREAM,
                                    FREERTOS_IPPROTO_TCP);
    configASSERT(xListenSocket != FREERTOS_INVALID_SOCKET);

    /* Bind to port */
    xBindAddress.sin_port = FreeRTOS_htons(IPERF_PORT);
    FreeRTOS_bind(xListenSocket, &xBindAddress, sizeof(xBindAddress));

    /* Listen for incoming connections */
    FreeRTOS_listen(xListenSocket, 1);

    printf("iperf: TCP server listening on port %d\r\n", IPERF_PORT);

    for (;;)
    {
        /* Accept client */
        xClientSocket = FreeRTOS_accept(xListenSocket,
                                        &xClientAddress, &xSize);
        if (xClientSocket == FREERTOS_INVALID_SOCKET) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        printf("iperf: client connected %d\r\n", xClientSocket);

        /* Read 1 byte mode from client */
        xBytes = FreeRTOS_recv(xClientSocket, &cMode, 1, 0);
        if (xBytes <= 0) {
            printf("iperf server: failed to read mode\n");
            FreeRTOS_closesocket(xClientSocket);
            continue;
        }
        printf("iperf server: mode '%c'\n", cMode);

        if (cMode == '@') {
            printf("iperf server: running in NORMAL mode (receive)\n");

            ulTotalBytes = 0;
            ulLastReportBytes = 0;
            xLastReport = alt_nticks();
            

            /* Receive loop */
            for (;;)
            {
                xBytes = FreeRTOS_recv(xClientSocket,
                                    pcBuffer, IPERF_BUFFER_SIZE, 0);


                if (xBytes <= 0) {
                    printf("iperf: client disconnected\r\n");
                    break;
                }

                ulTotalBytes += xBytes;


                 TickType_t xNow = alt_nticks();
                 TickType_t delta_ticks = xNow - xLastReport;

                    if (delta_ticks >= alt_ticks_per_second()) {   // 1 second

                        uint32_t ulDiff = ulTotalBytes - ulLastReportBytes;

                        uint32_t elapsed_ms =
                            (uint32_t)(delta_ticks * 1000ULL / alt_ticks_per_second());

                        uint32_t throughput_kbps =
                            (ulDiff * 8U) / elapsed_ms;  // bits/ms = kbps

                        printf("bytes=%" PRIu32 ", elapsed=%" PRIu32 " ms\r\n",
                               ulDiff, elapsed_ms);

                        printf("iperf: %" PRIu32 " kbps\r\n", throughput_kbps);
#if APP_DEBUG
                        dumpMACStats();
                        dump_tse_fifo_thresholds();
#endif
                        ulLastReportBytes = ulTotalBytes;
                        xLastReport = xNow;
                    }
       

                }

            }
        else if (cMode == 'D') {
            printf("iperf server: running in REVERSE mode (send)\n");

            ulTotalBytes = 0;
            ulLastReportBytes = 0;
            xLastReport = xTaskGetTickCount();

            for (;;)
            {
                xBytes = FreeRTOS_send(xClientSocket, pcBuffer, IPERF_BUFFER_SIZE, 0);
                if (xBytes <= 0) {
                    printf("iperf server: send failed\n");
                    break;
                }
                ulTotalBytes += xBytes;



                 TickType_t xNow = alt_nticks();
                 TickType_t delta_ticks = xNow - xLastReport;

                    if (delta_ticks >= alt_ticks_per_second()) {   // 1 second

                        uint32_t ulDiff = ulTotalBytes - ulLastReportBytes;

                        uint32_t elapsed_ms =
                            (uint32_t)(delta_ticks * 1000ULL / alt_ticks_per_second());

                        uint32_t throughput_kbps =
                            (ulDiff * 8U) / elapsed_ms;  // bits/ms = kbps

                        printf("bytes=%" PRIu32 ", elapsed=%" PRIu32 " ms\r\n",
                               ulDiff, elapsed_ms);

                        printf("iperf: %" PRIu32 " kbps\r\n", throughput_kbps);

                        ulLastReportBytes = ulTotalBytes;
                        xLastReport = xNow;
                    }


                #if 0
                if ((xNow - xStart) >= pdMS_TO_TICKS(IPERF_TEST_DURATION)) {
                    printf("iperf server: test duration ended\n");
                    break;
                }
                #endif
            }
        #if 1
        }
        else {
            printf("iperf server: unknown mode '%c'\n", cMode);
        }
        #endif
        FreeRTOS_closesocket(xClientSocket);
    }
}

#endif

/* Generates a locally administered random MAC address. */
void generate_random_mac(uint8_t * mac_address)
{
    // Set the locally administered bit (0x02 in the first octet)
    mac_address[0] = 0x02; 

    // Generate random bytes for the remaining 5 octets
    for ( int i = 1; i < 6; i++ )
    {
        mac_address[i] = (uint8_t)rand();
    }
}

#if PING_TASK_PORTING_STATUS 
/* Periodically sends ICMP ping requests to a fixed IP address. */
void vPingTestTask(void *pvParameters)
{
    uint32_t ucIPAddress;
    #if 0
    const uint8_t ucIPAddressToPing[4] = { 192, 168, 1, 50 };

    if(!FreeRTOS_inet_pton(FREERTOS_AF_INET, "192.168.1.50", &ucIPAddress))
        printf("IP address conversion failure\n");
    #else
    const uint8_t ucIPAddressToPing[4] = { 10, 0, 0, 2 };

    if(!FreeRTOS_inet_pton(FREERTOS_AF_INET, "10.0.0.2", &ucIPAddress))
        printf("IP address conversion failure\n");
    #endif
    const TickType_t xDelayBetweenPings = pdMS_TO_TICKS(100);

    xPingTaskHandle = xTaskGetCurrentTaskHandle();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (;;)
    {
        BaseType_t xResult = FreeRTOS_SendPingRequest(ucIPAddress,
                                                      1024,          // Ping data size
                                                      1000000);    // Timeout in ms

        if (xResult != pdFAIL)
        {
            printf("[PING] Request Sent: %u.%u.%u.%u Identifier %lu\n",
                   ucIPAddressToPing[0], ucIPAddressToPing[1],
                   ucIPAddressToPing[2], ucIPAddressToPing[3], xResult);
        }
        else
        {
            printf("[PING] Request Failed: %u.%u.%u.%u\n",
                   ucIPAddressToPing[0], ucIPAddressToPing[1],
                   ucIPAddressToPing[2], ucIPAddressToPing[3]);
        }
        vTaskDelay(xDelayBetweenPings);
    }
}
#endif

/* Provides a random number to the FreeRTOS+TCP stack. */
BaseType_t xApplicationGetRandomNumber( uint32_t *pulNumber )
{
    *pulNumber = ( uint32_t ) rand();
    return pdTRUE;
}

/* Returns an initial TCP sequence number for new connections. */
uint32_t ulApplicationGetNextSequenceNumber(
    uint32_t ulSourceAddress,
    uint16_t usSourcePort,
    uint32_t ulDestinationAddress,
    uint16_t usDestinationPort)
{
    return ( uint32_t ) rand();
}

/* Returns the device hostname used by the network stack. */
const char *pcApplicationHostnameHook(void)
{
    return "MyFreeRTOSDevice";
}

/* Handles DNS queries for the device hostname. */
BaseType_t xApplicationDNSQueryHook(const char *pcName)
{
    if (strcasecmp(pcName, pcApplicationHostnameHook()) == 0)
    {
        return pdPASS;
    }

    return pdFAIL;
}

/* Reports the result of received ping replies. */
void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier) {
    switch (eStatus) {
        case eSuccess:
            printf("[Ping] Reply received: Identifier %u\n", usIdentifier);
            break;
        case eInvalidChecksum:
            printf("[Ping] Reply received with invalid checksum: Identifier %u\n", usIdentifier);
            break;
        case eInvalidData:
            printf("[Ping] reply received with invalid data: Identifier %u\n", usIdentifier);
            break;
        default:
            printf("[Ping] Unknown ping reply status: Identifier %u\n", usIdentifier);
            break;
    }
}

/* Handles network up events and prints current IP configuration. */
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    char cBuffer[ 16 ];  

    if (eNetworkEvent == eNetworkUp)
    {
        FreeRTOS_printf(("Network is UP!\n"));

        uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;  
        FreeRTOS_GetAddressConfiguration(
            &ulIPAddress,
            &ulNetMask,
            &ulGatewayAddress,
            &ulDNSServerAddress
        );

        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );  
        printf( "IP Address: %s\n", cBuffer );  
  
        /* Convert the net mask to a string then print it out. */  
        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );  
        printf( "Subnet Mask: %s\n", cBuffer );  
  
        /* Convert the IP address of the gateway to a string then print it out. */  
        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );  
        printf( "Gateway IP Address: %s\n", cBuffer );  
  
        /* Convert the IP address of the DNS server to a string then print it out. */  
        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );  
        printf( "DNS server IP Address: %s\n", cBuffer ); 
    }
#if PING_TASK_PORTING_STATUS
    xTaskNotifyGive(xPingTaskHandle);
#endif

#if IPERF_TASK_PORTING_STATUS
    xTaskNotifyGive(xIperfTaskHandle);
#endif 


}

/* Initializes the network interface with static IP settings and a random MAC. */
BaseType_t vInitialiseNetworkInterface()
{
    #if 0
    const uint8_t ucIPAddress[ 4 ]  = { 192, 168, 1, 40 };
    const uint8_t ucGateway[ 4 ]    = { 192, 168, 1, 1 };
    #else
    const uint8_t ucIPAddress[ 4 ]  = { 10, 0, 0, 1 };
    const uint8_t ucGateway[ 4 ]    = { 10, 0, 0, 1 };
    #endif
    const uint8_t ucNetMask[ 4 ]    = { 255, 255, 255, 0 };
    const uint8_t ucDNSServer[ 4 ]  = { 8, 8, 8, 8 };
    uint8_t ucMACAddress[ 6 ];
    
    generate_random_mac(ucMACAddress);

	BaseType_t ret = FreeRTOS_IPInit(
        ucIPAddress,
        ucNetMask,
        ucGateway,
        ucDNSServer,
        ucMACAddress
    );
    return ret;
}

/* Entry point that initializes networking, creates tasks, and starts the scheduler. */
int main(void)
{
	printf("Hello FreeRTOS from main...\n");

#if 1
    if(!vInitialiseNetworkInterface()) {
        printf("Network initialization failure. System Halted\n");
        while(1);
    }
#endif

#if PING_TASK_PORTING_STATUS
    if (pdFAIL == xTaskCreate( vPingTestTask, "vPingTestTask", PING_TASK_STACKSIZE, NULL, PING_TASK_PRIORITY, &xPingTaskHandle)){
		printf("Ping Task creation fail!!!!\n");
	}
#endif

#if IPERF_TASK_PORTING_STATUS
    if (pdFAIL == xTaskCreate( vIperfServerTask, "iperfSvr", IPERF_STACK_SIZE, NULL, IPERF_TASK_PRIORITY, &xIperfTaskHandle)){
		printf("Iperf Task creation fail!!!!\n");
	}
#endif

	vTaskStartScheduler();

	for( ;; );
}
