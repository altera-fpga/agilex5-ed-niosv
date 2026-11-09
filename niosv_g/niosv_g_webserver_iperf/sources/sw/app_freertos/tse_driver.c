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

void phy_reset()
{
    alt_u16 phy_reg = 0x0;
    write_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_PAGE_ADDR, PAGE_0);
    phy_reg = read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_CONTROL_REG_0);
    phy_reg |= RESET;
    write_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_CONTROL_REG_0, phy_reg);
    vTaskDelay(pdMS_TO_TICKS(10000));
}

BaseType_t phy_init_rgmii()
{
    alt_u16 phy_reg = 0x0;
    alt_u16 status_reg = 0x0;
    alt_u16 real_time_link_status= 0x0;
    BaseType_t xReturn = pdFAIL;
    
    for (alt_32 i = 0; i < 10; i++) {

        //Reset
        phy_reset();

        //Setting Page 18 register 20 for system mode
        write_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_PAGE_ADDR, PAGE_18);
        //Configure RGMII.
        phy_reg = read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_GENERIC_CONTROL_REG_1);
        phy_reg &= ~(RGMII_MODE_0 | RGMII_MODE_1 | RGMII_MODE_2);
        write_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_GENERIC_CONTROL_REG_1, phy_reg);

        //Setting Page 0 register 0 setting to 1G speed
        write_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_PAGE_ADDR, PAGE_0);
        phy_reg = read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_CONTROL_REG_0);
        //Set 1 Gbps speed
        phy_reg |= SPEED_SELECTION0;
        phy_reg &= ~SPEED_SELECTION1;
        //Set Duplex mode
        phy_reg |= DUPLEX_MODE;
        write_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_CONTROL_REG_0, phy_reg);

        //RGMII timing control
        write_phy_register (TSE_MAC_BASE_ADDRESS, RGMII_PAGE_ADDR, PAGE_2);
        phy_reg = read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_MAC_SPECIFIC_CONTROL_REG_2);
        phy_reg |= RGMII_TX_TIMING_CONTROL;
        phy_reg &= ~RGMII_RX_TIMING_CONTROL;
        phy_reg |= MAC_SPEED_1GBPS;
        write_phy_register (TSE_MAC_BASE_ADDRESS, RGMII_MAC_SPECIFIC_CONTROL_REG_2, phy_reg);

        //Reset
        phy_reset();

        status_reg = read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_STATUS_REG_0);
        if (status_reg & AUTO_NEGOTIATION_COMPLETE) {
            printf("Auto-negotiation successful\n");
            xReturn = pdPASS;
        } else {
            printf("Auto-negotiation failed\n");
            xReturn = pdFAIL;
        }
        real_time_link_status = read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_SPECIFIC_STATUS_REG_1);

        if (tse_phy_link_up()) {
            printf("Link up successful\n");
            xReturn = pdPASS;
            break;
        } else {
            printf("Link up failed. Attempt %ld\n", i);
            xReturn = pdFAIL;
        }
    }

    if (real_time_link_status & SPEED_DUPLEX_RESOLVED) {
        printf("Speed and duplex resolved\n");
        xReturn = pdPASS;
    } else {
        printf("Speed and duplex not resolved\n");
        xReturn = pdFAIL;
    }

    if (((real_time_link_status & SPEED_MASK) >> SPEED_SHIFT) == SPEED_1GBPS) {
        printf("Speed is 1 Gbps\n");
        xReturn = pdPASS;
    } else {
        printf("Speed is not set to 1 Gbps\n");
        xReturn = pdFAIL;
    }
    return xReturn;
}

void tse_mac_init(MACAddress_t mac_address)
{
    alt_u32 cmd_cfg = TSE_MAC_REGISTERS->command_config;

	//MAC Configuration Register Initialization
	cmd_cfg &= ~(MAC_CMDCFG_TX_ENA | MAC_CMDCFG_RX_ENA);
    cmd_cfg |= MAC_CMDCFG_CNTL_FRM_ENA;
    
    TSE_MAC_REGISTERS->command_config = cmd_cfg;

    // MAC FIFO Configuration
    TSE_MAC_REGISTERS->tx_section_empty = ALTERA_TSE_TX_SECTION_EMPTY;
    TSE_MAC_REGISTERS->tx_almost_full = ALTERA_TSE_TX_ALMOST_FULL;
    TSE_MAC_REGISTERS->tx_almost_empty = ALTERA_TSE_TX_ALMOST_EMPTY;
    TSE_MAC_REGISTERS->rx_section_empty = ALTERA_TSE_RX_SECTION_EMPTY;
    TSE_MAC_REGISTERS->rx_almost_full = ALTERA_TSE_RX_ALMOST_FULL;
    TSE_MAC_REGISTERS->rx_almost_empty = ALTERA_TSE_RX_ALMOST_EMPTY;
    TSE_MAC_REGISTERS->tx_section_full = ALTERA_TSE_TX_SECTION_FULL;
    TSE_MAC_REGISTERS->rx_section_full = ALTERA_TSE_RX_SECTION_FULL;

    // MAC Address Configuration
    uint32_t mac_addr_high = ( (uint32_t)mac_address.ucBytes[5] << 8 ) |
                             ( (uint32_t)mac_address.ucBytes[4] );

    uint32_t mac_addr_low  = ( (uint32_t)mac_address.ucBytes[3] << 24 ) |
                             ( (uint32_t)mac_address.ucBytes[2] << 16 ) |
                             ( (uint32_t)mac_address.ucBytes[1] << 8  ) |
                             ( (uint32_t)mac_address.ucBytes[0] );
                         
    TSE_MAC_REGISTERS->mac_addr_0 = mac_addr_low;
	TSE_MAC_REGISTERS->mac_addr_1 = mac_addr_high;

    // MAC Function Configuration
    TSE_MAC_REGISTERS->frm_length = ALTERA_TSE_FRAME_LENGTH;
    TSE_MAC_REGISTERS->tx_ipg_length = ALTERA_TSE_TX_IPG_LENGTH;
    TSE_MAC_REGISTERS->pause_quanta = ALTERA_TSE_PAUSE_QUANTA;

    cmd_cfg = TSE_MAC_REGISTERS->command_config;

    cmd_cfg |= MAC_CMDCFG_ETH_SPEED;
    cmd_cfg &= ~MAC_CMDCFG_HD_ENA;
    cmd_cfg |= MAC_CMDCFG_PAD_EN;
    cmd_cfg &= ~MAC_CMDCFG_CRC_FWD;
    cmd_cfg |= MAC_CMDCFG_TX_ADDR_INS;
    cmd_cfg &= ~MAC_CMDCFG_TX_ADDR_SEL(111);

    TSE_MAC_REGISTERS->command_config = cmd_cfg;

    //SW Reset
    cmd_cfg = TSE_MAC_REGISTERS->command_config;
    cmd_cfg |= MAC_CMDCFG_SW_RESET;
    TSE_MAC_REGISTERS->command_config = cmd_cfg;

	while (1) {
        cmd_cfg = TSE_MAC_REGISTERS->command_config;
        if ((cmd_cfg & MAC_CMDCFG_SW_RESET) == 0) {
            printf("TSE MAC SW Reset complete\n");
            break;
        }
    }

#if ENABLE_MAC_LOOPBACK
    //MAC loopback
    cmd_cfg = TSE_MAC_REGISTERS->command_config;
    cmd_cfg |= MAC_CMDCFG_LOOP_ENA;
    TSE_MAC_REGISTERS->command_config = cmd_cfg;

    vTaskDelay(pdMS_TO_TICKS(1000));
#endif

    cmd_cfg = TSE_MAC_REGISTERS->command_config;
    cmd_cfg |= MAC_CMDCFG_TX_ENA;
    cmd_cfg |= MAC_CMDCFG_RX_ENA;
    TSE_MAC_REGISTERS->command_config = cmd_cfg;
    while (1) {
        cmd_cfg = TSE_MAC_REGISTERS->command_config;
        if ((cmd_cfg & (MAC_CMDCFG_TX_ENA | MAC_CMDCFG_RX_ENA)) != 0) {
            printf("TSE MAC Tx and Rx are enabled\n");
            break;
        }
    }
    TSE_MAC_REGISTERS->tx_cmd_stat = 0x0;
    TSE_MAC_REGISTERS->rx_cmd_stat = 0x0;
}

BaseType_t tse_mac_phy_init(MACAddress_t mac_addr)
{
    BaseType_t xReturn = pdFAIL;
	//External PHY Initialization using MDIO
	TSE_MAC_REGISTERS->mdio_phy0_addr = EXTERNAL_PHY_ADDRESS;

    xReturn = phy_init_rgmii();
    tse_mac_init(mac_addr);

    printf("MAC ID 1 = 0x%x\n", TSE_MAC_REGISTERS->mac_id_1);
    printf("MAC ID 2 = 0x%x\n", TSE_MAC_REGISTERS->mac_id_2);
    printf("PHY ID 1 = 0x%x\n", read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_PHY_ID_1));
    printf("PHY ID 2 = 0x%x\n", read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_PHY_ID_2));

    return xReturn;
}

BaseType_t tse_phy_link_up() {
    return (((read_phy_register(TSE_MAC_BASE_ADDRESS, RGMII_COPPER_SPECIFIC_STATUS_REG_1)) & LINK_STATUS_MASK) == LINK_STATUS_MASK);
}

void dumpMACStats() {
    printf("MAC Stats dump\n");
    printf("frames_transmitted_ok = 0x%X\n", TSE_MAC_REGISTERS->frames_transmitted_ok);
    printf("frames_received_ok = 0x%X\n", TSE_MAC_REGISTERS->frames_received_ok);
    printf("frames_check_sequence_errors = 0x%X\n", TSE_MAC_REGISTERS->frames_check_sequence_errors);
    printf("alignment_errors = 0x%X\n", TSE_MAC_REGISTERS->alignment_errors);
    printf("octets_transmitted_ok = 0x%X\n", TSE_MAC_REGISTERS->octets_transmitted_ok);
    printf("octets_received_ok = 0x%X\n", TSE_MAC_REGISTERS->octets_received_ok);
    printf("tx_pause_mac_ctrl_frames = 0x%X\n", TSE_MAC_REGISTERS->tx_pause_mac_ctrl_frames);
    printf("rx_pause_mac_ctrl_frames = 0x%X\n", TSE_MAC_REGISTERS->rx_pause_mac_ctrl_frames);
    printf("if_in_errors = 0x%X\n", TSE_MAC_REGISTERS->if_in_errors);
    printf("if_out_errors = 0x%X\n", TSE_MAC_REGISTERS->if_out_errors);
    printf("if_in_ucast_pkts = 0x%X\n", TSE_MAC_REGISTERS->if_in_ucast_pkts);
    printf("if_in_multicast_pkts = 0x%X\n", TSE_MAC_REGISTERS->if_in_multicast_pkts); 
    printf("if_in_broadcast_pkts = 0x%X\n", TSE_MAC_REGISTERS->if_in_broadcast_pkts);
    printf("if_out_discards = 0x%X\n", TSE_MAC_REGISTERS->if_out_discards);
    printf("if_out_ucast_pkts = 0x%X\n", TSE_MAC_REGISTERS->if_out_ucast_pkts);
    printf("if_out_multicast_pkts = 0x%X\n", TSE_MAC_REGISTERS->if_out_multicast_pkts);
    printf("if_out_broadcast_pkts = 0x%X\n", TSE_MAC_REGISTERS->if_out_broadcast_pkts);
    printf("ether_stats_drop_events = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_drop_events);
    printf("ether_stats_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_octets);
    printf("ether_stats_pkts = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts);
    printf("ether_stats_undersize_pkts = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_undersize_pkts);
    printf("ether_stats_oversize_pkts = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_oversize_pkts);
    printf("ether_stats_pkts_64_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_64_octets);
    printf("ether_stats_pkts_65to127_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_65to127_octets);
    printf("ether_stats_pkts_128to255_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_128to255_octets);
    printf("ether_stats_pkts_256to511_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_256to511_octets);
    printf("ether_stats_pkts_512to1023_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_512to1023_octets);
    printf("ether_stats_pkts_1024to1518_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_1024to1518_octets); 
    printf("ether_stats_pkts_1519tox_octets = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_pkts_1519tox_octets);
    printf("ether_stats_jabbers = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_jabbers);
    printf("ether_stats_fragments = 0x%X\n", TSE_MAC_REGISTERS->ether_stats_fragments);
}
