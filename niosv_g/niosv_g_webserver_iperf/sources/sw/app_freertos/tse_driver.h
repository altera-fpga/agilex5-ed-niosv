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

#ifndef TSE_DRIVER_H
#define TSE_DRIVER_H

#include "FreeRTOS_IP.h"

#define BIT(x)  (1 << (x))
#define GET_BIT_VALUE(v, bit)           (((v) >> (bit)) & 0x1)
#define TSE_MAC_REGISTERS ((volatile struct altera_tse_mac *)TSE_MAC_BASE_ADDRESS)
#define CALCULATE_PHY_REGISTER_ADDRESS(base, reg_offset) ((base) + 4 * (0x80 + (reg_offset)))

#define TSE_MAC_BASE_ADDRESS SYS_TSE_BASE
#define EXTERNAL_PHY_ADDRESS 0X200
#define MAX_FIFO_SIZE 8192

#define SPEED_SHIFT			14
#define SPEED_1GBPS			0x2

#define ENABLE_MAC_LOOPBACK	0

/* Rx FIFO default settings */
#define ALTERA_TSE_RX_SECTION_EMPTY     MAX_FIFO_SIZE - 16
#define ALTERA_TSE_RX_SECTION_FULL      16
#define ALTERA_TSE_RX_ALMOST_EMPTY      8
#define ALTERA_TSE_RX_ALMOST_FULL       8

/* Tx FIFO default settings */
#define ALTERA_TSE_TX_SECTION_EMPTY     MAX_FIFO_SIZE - 16
#define ALTERA_TSE_TX_SECTION_FULL      16
#define ALTERA_TSE_TX_ALMOST_EMPTY      8
#define ALTERA_TSE_TX_ALMOST_FULL       3

/* MAC function configuration default settings */
#define ALTERA_TSE_FRAME_LENGTH			1518
#define ALTERA_TSE_TX_IPG_LENGTH        12
#define ALTERA_TSE_PAUSE_QUANTA         0xFFFF

/* MAC Command_Config Register Bit Definitions
 */
#define MAC_CMDCFG_TX_ENA                       BIT(0)
#define MAC_CMDCFG_RX_ENA                       BIT(1)
#define MAC_CMDCFG_XON_GEN                      BIT(2)
#define MAC_CMDCFG_ETH_SPEED                    BIT(3)
#define MAC_CMDCFG_PROMIS_EN                    BIT(4)
#define MAC_CMDCFG_PAD_EN                       BIT(5)
#define MAC_CMDCFG_CRC_FWD                      BIT(6)
#define MAC_CMDCFG_PAUSE_FWD                    BIT(7)
#define MAC_CMDCFG_PAUSE_IGNORE                 BIT(8)
#define MAC_CMDCFG_TX_ADDR_INS                  BIT(9)
#define MAC_CMDCFG_HD_ENA                       BIT(10)
#define MAC_CMDCFG_EXCESS_COL                   BIT(11)
#define MAC_CMDCFG_LATE_COL                     BIT(12)
#define MAC_CMDCFG_SW_RESET                     BIT(13)
#define MAC_CMDCFG_MHASH_SEL                    BIT(14)
#define MAC_CMDCFG_LOOP_ENA                     BIT(15)
#define MAC_CMDCFG_TX_ADDR_SEL(v)               (((v) & 0x7) << 16)
#define MAC_CMDCFG_MAGIC_ENA                    BIT(19)
#define MAC_CMDCFG_SLEEP                        BIT(20)
#define MAC_CMDCFG_WAKEUP                       BIT(21)
#define MAC_CMDCFG_XOFF_GEN                     BIT(22)
#define MAC_CMDCFG_CNTL_FRM_ENA                 BIT(23)
#define MAC_CMDCFG_NO_LGTH_CHECK                BIT(24)
#define MAC_CMDCFG_ENA_10                       BIT(25)
#define MAC_CMDCFG_RX_ERR_DISC                  BIT(26)
#define MAC_CMDCFG_DISABLE_READ_TIMEOUT         BIT(27)
#define MAC_CMDCFG_CNT_RESET                    BIT(31)

static inline void write_phy_register(alt_u32 base, alt_u32 reg_offset, alt_u16 value) {
    volatile alt_u16 *reg_address = (volatile alt_u16 *)CALCULATE_PHY_REGISTER_ADDRESS(base, reg_offset);
    *reg_address = value;
}

static inline alt_u16 read_phy_register(alt_u32 base, alt_u32 reg_offset) {
    volatile alt_u16 *reg_address = (volatile alt_u16 *)CALCULATE_PHY_REGISTER_ADDRESS(base, reg_offset);
    return *reg_address;
}

BaseType_t tse_mac_phy_init(MACAddress_t mac_address);
BaseType_t tse_phy_link_up(void);

/* MAC register Space. Note that some of these registers may or may not be
 * present depending upon options chosen by the user when the core was
 * configured and built. Please consult the Altera Triple Speed Ethernet User
 * Guide for details.
 */
struct altera_tse_mac {
	/* Bits 15:0: MegaCore function revision (0x0800). Bit 31:16: Customer
	 * specific revision
	 */
	int megacore_revision;
	/* Provides a memory location for user applications to test the device
	 * memory operation.
	 */
	int scratch_pad;
	/* The host processor uses this register to control and configure the
	 * MAC block
	 */
	int command_config;
	/* 32-bit primary MAC address word 0 bits 0 to 31 of the primary
	 * MAC address
	 */
	int mac_addr_0;
	/* 32-bit primary MAC address word 1 bits 32 to 47 of the primary
	 * MAC address
	 */
	int mac_addr_1;
	/* 14-bit maximum frame length. The MAC receive logic */
	int frm_length;
	/* The pause quanta is used in each pause frame sent to a remote
	 * Ethernet device, in increments of 512 Ethernet bit times
	 */
	int pause_quanta;
	/* 12-bit receive FIFO section-empty threshold */
	int rx_section_empty;
	/* 12-bit receive FIFO section-full threshold */
	int rx_section_full;
	/* 12-bit transmit FIFO section-empty threshold */
	int tx_section_empty;
	/* 12-bit transmit FIFO section-full threshold */
	int tx_section_full;
	/* 12-bit receive FIFO almost-empty threshold */
	int rx_almost_empty;
	/* 12-bit receive FIFO almost-full threshold */
	int rx_almost_full;
	/* 12-bit transmit FIFO almost-empty threshold */
	int tx_almost_empty;
	/* 12-bit transmit FIFO almost-full threshold */
	int tx_almost_full;
	/* MDIO address of PHY Device 0. Bits 0 to 4 hold a 5-bit PHY address */
	int mdio_phy0_addr;
	/* MDIO address of PHY Device 1. Bits 0 to 4 hold a 5-bit PHY address */
	int mdio_phy1_addr;

	/* Bit[15:0]—16-bit holdoff quanta */
	int holdoff_quant;

	/* only if 100/1000 BaseX PCS, reserved otherwise */
	int reserved1[5];

	/* Minimum IPG between consecutive transmit frame in terms of bytes */
	int tx_ipg_length;

	/* IEEE 802.3 oEntity Managed Object Support */

	/* The MAC addresses */
	int mac_id_1;
	int mac_id_2;

	/* Number of frames transmitted without error including pause frames */
	int frames_transmitted_ok;
	/* Number of frames received without error including pause frames */
	int frames_received_ok;
	/* Number of frames received with a CRC error */
	int frames_check_sequence_errors;
	/* Frame received with an alignment error */
	int alignment_errors;
	/* Sum of payload and padding octets of frames transmitted without
	 * error
	 */
	int octets_transmitted_ok;
	/* Sum of payload and padding octets of frames received without error */
	int octets_received_ok;

	/* IEEE 802.3 oPausedEntity Managed Object Support */

	/* Number of transmitted pause frames */
	int tx_pause_mac_ctrl_frames;
	/* Number of Received pause frames */
	int rx_pause_mac_ctrl_frames;

	/* IETF MIB (MIB-II) Object Support */

	/* Number of frames received with error */
	int if_in_errors;
	/* Number of frames transmitted with error */
	int if_out_errors;
	/* Number of valid received unicast frames */
	int if_in_ucast_pkts;
	/* Number of valid received multicasts frames (without pause) */
	int if_in_multicast_pkts;
	/* Number of valid received broadcast frames */
	int if_in_broadcast_pkts;
	int if_out_discards;//reserved
	/* The number of valid unicast frames transmitted */
	int if_out_ucast_pkts;
	/* The number of valid multicast frames transmitted,
	 * excluding pause frames
	 */
	int if_out_multicast_pkts;
	int if_out_broadcast_pkts;

	/* IETF RMON MIB Object Support */

	/* Counts the number of dropped packets due to internal errors
	 * of the MAC client.
	 */
	int ether_stats_drop_events;
	/* Total number of bytes received. Good and bad frames. */
	int ether_stats_octets;
	/* Total number of packets received. Counts good and bad packets. */
	int ether_stats_pkts;
	/* Number of packets received with less than 64 bytes. */
	int ether_stats_undersize_pkts;
	/* The number of frames received that are longer than the
	 * value configured in the frm_length register
	 */
	int ether_stats_oversize_pkts;
	/* Number of received packet with 64 bytes */
	int ether_stats_pkts_64_octets;
	/* Frames (good and bad) with 65 to 127 bytes */
	int ether_stats_pkts_65to127_octets;
	/* Frames (good and bad) with 128 to 255 bytes */
	int ether_stats_pkts_128to255_octets;
	/* Frames (good and bad) with 256 to 511 bytes */
	int ether_stats_pkts_256to511_octets;
	/* Frames (good and bad) with 512 to 1023 bytes */
	int ether_stats_pkts_512to1023_octets;
	/* Frames (good and bad) with 1024 to 1518 bytes */
	int ether_stats_pkts_1024to1518_octets;

	/* Any frame length from 1519 to the maximum length configured in the
	 * frm_length register, if it is greater than 1518
	 */
	int ether_stats_pkts_1519tox_octets;
	/* Too long frames with CRC error */
	int ether_stats_jabbers;
	/* Too short frames with CRC error */
	int ether_stats_fragments;

	int reserved2;

	/* FIFO control register */
	int tx_cmd_stat;
	int rx_cmd_stat;

	/* Extended Statistics Counters */
	int msb_octets_transmitted_ok;
	int msb_octets_received_ok;
	int msb_ether_stats_octets;

	int reserved3;

	/* Multicast address resolution table, mapped in the controller address
	 * space
	 */
	int hash_table[64];

	/* 4 Supplemental MAC Addresses */
	int supp_mac_addr_0_0;
	int supp_mac_addr_0_1;
	int supp_mac_addr_1_0;
	int supp_mac_addr_1_1;
	int supp_mac_addr_2_0;
	int supp_mac_addr_2_1;
	int supp_mac_addr_3_0;
	int supp_mac_addr_3_1;

	int reserved4[8];

	/* IEEE 1588v2 Feature */
	int tx_period;
	int tx_adjust_fns;
	int tx_adjust_ns;
	int rx_period;
	int rx_adjust_fns;
	int rx_adjust_ns;

	int reserved5[42];
};

enum phy_reg_offset {
	RGMII_COPPER_CONTROL_REG_0 = 0X00,
	RGMII_COPPER_STATUS_REG_0 = 0X01,
	RGMII_PHY_ID_1 = 0x02,
	RGMII_PHY_ID_2 = 0x03,
	RGMII_COPPER_SPECIFIC_STATUS_REG_1 = 0X11,
	RGMII_GENERIC_CONTROL_REG_1 = 0X14,
    RGMII_MAC_SPECIFIC_CONTROL_REG_2 = 0X15,
    RGMII_PAGE_ADDR = 0X16,
};

enum rgmii_copper_control_reg {
	SPEED_SELECTION0 = BIT(6),
	SPEED_SELECTION1 = BIT(13),
	DUPLEX_MODE = BIT(8),
	RESTART_AUTO_NEGOTIATION = BIT(9),
	ISOLATE = BIT(10),
	POWERDOWN = BIT(11),
	AUTO_NEGOTIATION_ENABLE = BIT(12),
	SD_LOOPBACK = BIT(14),
	RESET = BIT(15),
};

enum rgmii_copper_status_reg {
	AUTO_NEGOTIATION_ABILITY = BIT(3),
	AUTO_NEGOTIATION_COMPLETE = BIT(5),
};

enum rgmii_copper_specific_status_reg {
	GLOBAL_LINK_STATUS = BIT(3),
	COPPER_LINK_STATUS = BIT(10),
	SPEED_DUPLEX_RESOLVED = BIT(11),
	SPEED_1GBPS_0 = BIT(14),
	SPEED_1GBPS_1 = BIT(15),
};

enum rgmii_generic_control_reg {
	RGMII_MODE_0 = BIT(0),
	RGMII_MODE_1 = BIT(1),
	RGMII_MODE_2 = BIT(2),
};

enum rgmii_mac_specific_control_reg_2 {
	RGMII_TX_TIMING_CONTROL = BIT(4),
	RGMII_RX_TIMING_CONTROL = BIT(5),
	MAC_SPEED_1GBPS = BIT(6),
};

enum PAGE_NUM {
	PAGE_0 = 0x0,
	PAGE_2 = 0x2,
	PAGE_18 = 0x12,
};

#define LINK_STATUS_MASK	(GLOBAL_LINK_STATUS | COPPER_LINK_STATUS)
#define SPEED_MASK			(SPEED_1GBPS_0 | SPEED_1GBPS_1)

#endif
