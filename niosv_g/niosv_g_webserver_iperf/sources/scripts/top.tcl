# Copyright (C) 2026  Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions 
# and other software and tools, and any partner logic 
# functions, and any output files from any of the foregoing 
# (including device programming or simulation files), and any 
# associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License 
# Subscription Agreement, the Altera Quartus Prime License Agreement,
# the Altera IP License Agreement, or other applicable license
# agreement, including, without limitation, that your use is for
# the sole purpose of programming logic devices manufactured by
# Altera and sold by Altera or its authorized distributors.  Please
# refer to the Altera Software License Subscription Agreements 
# on the Quartus Prime software download page.

# Quartus Prime: Generate Tcl File for Project
# File: top.tcl
# Generated on: Tue Sep 29 07:57:09 2026

# Load Quartus Prime Tcl Project package
package require ::quartus::project

set need_to_close_project 0
set make_assignments 1

# Check that the right project is open
if {[is_project_open]} {
	if {[string compare $quartus(project) "top"]} {
		puts "Project top is not open"
		set make_assignments 0
	}
} else {
	# Only open if not already open
	if {[project_exists top]} {
		project_open -revision top top
	} else {
		project_new -revision top top
	}
	set need_to_close_project 1
}

# Make assignments
if {$make_assignments} {
	set_global_assignment -name TOP_LEVEL_ENTITY top
	set_global_assignment -name ORIGINAL_QUARTUS_VERSION 17.1.0
	set_global_assignment -name PROJECT_CREATION_TIME_DATE "20:40:19  NOVEMBER 23, 2017"
	set_global_assignment -name LAST_QUARTUS_VERSION "26.3.0 Pro Edition"
	set_global_assignment -name FAMILY "Agilex 5"
	set_global_assignment -name DEVICE A5ED065BB32AE6SR0
	set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
	set_global_assignment -name ERROR_CHECK_FREQUENCY_DIVISOR 256
	set_global_assignment -name EDA_SIMULATION_TOOL "ModelSim-Altera (Verilog)"
	set_global_assignment -name EDA_TIME_SCALE "1 ps" -section_id eda_simulation
	set_global_assignment -name EDA_OUTPUT_DATA_FORMAT "VERILOG HDL" -section_id eda_simulation
	set_global_assignment -name PRESERVE_UNUSED_XCVR_CHANNEL ON
	set_global_assignment -name GENERATE_COMPRESSED_SOF ON
	set_global_assignment -name AUTO_RESTART_CONFIGURATION OFF
	set_global_assignment -name STRATIXV_CONFIGURATION_SCHEME "AVST X8"
	set_global_assignment -name ON_CHIP_BITSTREAM_DECOMPRESSION OFF
	set_global_assignment -name USE_CONF_DONE SDM_IO5
	set_global_assignment -name USE_HPS_COLD_RESET SDM_IO12
	set_global_assignment -name DEVICE_INITIALIZATION_CLOCK OSC_CLK_1_125MHZ
	set_global_assignment -name PWRMGT_VOLTAGE_OUTPUT_FORMAT "LINEAR FORMAT"
	set_global_assignment -name PWRMGT_LINEAR_FORMAT_N "-12"
	set_global_assignment -name POWER_APPLY_THERMAL_MARGIN ADDITIONAL
	set_global_assignment -name BOARD "Agilex 5 FPGA E-Series 065B Premium Development Kit DK-A5E065BB32AES1"
	set_global_assignment -name POWER_THERMAL_SOLVER_MODE FIND_MAX_TJ
	set_global_assignment -name ENABLE_SIGNALTAP ON
	set_global_assignment -name SEED 1
	set_global_assignment -name SDC_FILE jtag.sdc
	set_global_assignment -name IP_FILE sys_pll.ip
	set_global_assignment -name VERILOG_FILE top.v
	set_global_assignment -name SDC_FILE top.sdc
	set_global_assignment -name IP_FILE reset_release.ip
	set_global_assignment -name IP_FILE issp.ip
	set_global_assignment -name VDS_FILE src/vds/qsys_top/qsys_top.vds -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_intel_onchip_memory_2.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_clock_bridge_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_intel_niosv_g_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_reset_bridge_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_intel_onchip_memory_1.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_intel_onchip_memory_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_jtag_uart_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_pio_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_sysid_qsys_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_intel_eth_tse_0.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_msgdma_1.ip -tag "vds::qsys_top"
	set_global_assignment -name VDS_IP_FILE src/vds/qsys_top/ip/qsys_top_msgdma_0.ip -tag "vds::qsys_top"
	# Use High Performance Effort optimization mode when optimizing for performance. Superior Performance with Maximum Placement Effort optimization mode has been deprecated and will be removed in 27.1.1.
	set_global_assignment -name OPTIMIZATION_MODE "HIGH PERFORMANCE EFFORT" -comment "Use High Performance Effort optimization mode when optimizing for performance. Superior Performance with Maximum Placement Effort optimization mode has been deprecated and will be removed in 27.1.1."
	set_global_assignment -name OPTIMIZATION_TECHNIQUE SPEED
	set_global_assignment -name MUX_RESTRUCTURE OFF
	set_global_assignment -name GLOBAL_PLACEMENT_EFFORT "MAXIMUM EFFORT"
	set_instance_assignment -name PARTITION_COLOUR 4289977599 -to top -entity top
	set_instance_assignment -name PARTITION_COLOUR 4285964287 -to auto_fab_0 -entity top
	set_location_assignment PIN_BM71 -to CLK_125M
	set_instance_assignment -name IO_STANDARD "1.1-V TRUE DIFFERENTIAL SIGNALING" -to CLK_125M -entity top
	set_location_assignment PIN_BM59 -to user_led[0]
	set_location_assignment PIN_BH59 -to user_led[1]
	set_location_assignment PIN_BH62 -to user_led[2]
	set_location_assignment PIN_BK59 -to user_led[3]
	set_instance_assignment -name IO_STANDARD "1.1 V" -to user_led[0] -entity top
	set_instance_assignment -name IO_STANDARD "1.1 V" -to user_led[1] -entity top
	set_instance_assignment -name IO_STANDARD "1.1 V" -to user_led[2] -entity top
	set_instance_assignment -name IO_STANDARD "1.1 V" -to user_led[3] -entity top
	# IOBANK_6D
	set_location_assignment PIN_A39 -to MDIO -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B26 -to MDC -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_D34 -to phy_resetn -comment IOBANK_6D
	set_instance_assignment -name IO_STANDARD "1.8-V LVCMOS" -to MDC -entity top
	set_instance_assignment -name IO_STANDARD "1.8-V LVCMOS" -to MDIO -entity top
	set_instance_assignment -name IO_STANDARD "1.8-V LVCMOS" -to phy_resetn -entity top
	# IOBANK_6D
	set_location_assignment PIN_A30 -to rgmii_in[0] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B30 -to rgmii_in[1] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_A33 -to rgmii_in[2] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_A35 -to rgmii_in[3] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_A8 -to rgmii_out[0] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B4 -to rgmii_out[1] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_A11 -to rgmii_out[2] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B11 -to rgmii_out[3] -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B14 -to rgmii_txclk -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_A14 -to rgmii_txctl -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B23 -to rgmii_rxclk -comment IOBANK_6D
	# IOBANK_6D
	set_location_assignment PIN_B20 -to rgmii_rxctl -comment IOBANK_6D
	set_instance_assignment -name IO_STANDARD "1.8-V LVCMOS" -to rgmii* -entity top
	set_instance_assignment -name INPUT_DELAY_CHAIN 45 -to rx_ctrl -entity top
	set_instance_assignment -name INPUT_DELAY_CHAIN 45 -to rgmii_in[0] -entity top
	set_instance_assignment -name INPUT_DELAY_CHAIN 45 -to rgmii_in[1] -entity top
	set_instance_assignment -name INPUT_DELAY_CHAIN 45 -to rgmii_in[2] -entity top
	set_instance_assignment -name INPUT_DELAY_CHAIN 45 -to rgmii_in[3] -entity top

	# Commit assignments
	export_assignments

	# Close project
	if {$need_to_close_project} {
		project_close
	}
}
