# (C) 2001-2024 Intel Corporation. All rights reserved.
# Your use of Intel Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files from any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Intel Program License Subscription 
# Agreement, Intel FPGA IP License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Intel and sold by 
# Intel or its authorized distributors.  Please refer to the applicable 
# agreement for further details.


# (C) 2001-2023 Intel Corporation. All rights reserved.
# Your use of Intel Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files from any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Intel Program License Subscription 
# Agreement, Intel FPGA IP License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Intel and sold by 
# Intel or its authorized distributors.  Please refer to the applicable 
# agreement for further details.


# Copyright (C) 2001-2022 Intel Corporation 
#
# This code and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# code or the related documents without Intel's prior written permission
#
# This code and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# Revision    Date         Quartus version      Comment 
# ========    ====         ===============      =======
# 1.0         28-Jul-20    20.2                 Initial release
# 

set_time_format -unit ns -decimal_places 3

#**************************************************************
# Create Clock
#**************************************************************
#create_clock -name "CLK_125M" -period "125 MHz" [get_ports "CLK_125M"]
create_clock -name CLK_125M -period 8.0 [get_ports CLK_125M]
create_clock -name {mac_clk_125_virtual} -period 8.00
create_clock -name rgmii_rxclk -period 8.0 [get_ports rgmii_rxclk]


#**************************************************************
# Set Clock Uncertainty
#**************************************************************
derive_clock_uncertainty

#**************************************************************
# Sourcing JTAG related SDC
#**************************************************************
source ./jtag.sdc

#**************************************************************
# Set False Path
#**************************************************************
set_false_path -from RESET_N
set_false_path -from * -to PHY_RESET_N
set_false_path -from * -to [get_ports LED_*]

# Soft-CDR path and timing is not critical
set_false_path -from RXP

#**************************************************************
# Ethernet MDIO interface
#**************************************************************
set_output_delay  -clock [ get_clocks "REF_CLK" ] 2   [ get_ports {MDC} ]
set_input_delay   -clock [ get_clocks "REF_CLK" ] 2   [ get_ports {MDIO} ]
set_output_delay  -clock [ get_clocks "REF_CLK" ] 2   [ get_ports {MDIO} ]


###############################
#####   Set Input delay   #####
###############################
# 3665.84 / (6000 mil per 1ns) = 0.61ns
set clk_trace_min 0.61
set clk_trace_max 0.61
 

set rxctl_trace_min 0.608
set rxctl_trace_max 0.608
set rxd0_trace_min 0.61
set rxd0_trace_max 0.61
set rxd1_trace_min 0.62
set rxd1_trace_max 0.62
set rxd2_trace_min 0.71
set rxd2_trace_max 0.71
set rxd3_trace_min 0.62
set rxd3_trace_max 0.62
 
set ext_tco_min 0.3
set ext_tco_max 0.6
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxctl_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_rx_control]
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxctl_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_rx_control]
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxctl_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_rx_control] -clock_fall -add_delay
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxctl_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_rx_control] -clock_fall -add_delay
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd0_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[0]]
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd0_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[0]]
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd0_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[0]] -clock_fall -add_delay
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd0_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[0]] -clock_fall -add_delay
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd1_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[1]]
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd1_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[1]]
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd1_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[1]] -clock_fall -add_delay
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd1_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[1]] -clock_fall -add_delay
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd2_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[2]]
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd2_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[2]]
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd2_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[2]] -clock_fall -add_delay
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd2_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[2]] -clock_fall -add_delay
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd3_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[3]]
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd3_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[3]]
 
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -max [expr {$rxd3_trace_max - $clk_trace_min + $ext_tco_max}] [get_ports i_rgmii_in[3]] -clock_fall -add_delay
set_input_delay -clock [get_clocks mac_clk_125_virtual]  -source_latency_included -min [expr {$rxd3_trace_min - $clk_trace_max + $ext_tco_min}] [get_ports i_rgmii_in[3]] -clock_fall -add_delay 
 
## To make sure to consider the immediate posedge at destination 
set_multicycle_path 0 -setup -end -rise_from [get_clocks mac_clk_125_virtual] -rise_to [get_clocks rgmii_rx_clk]
set_multicycle_path 0 -setup -end -fall_from [get_clocks mac_clk_125_virtual] -fall_to [get_clocks rgmii_rx_clk]


set_false_path -fall_from [get_clocks mac_clk_125_virtual] -rise_to [get_clocks rgmii_rxclk] -setup
set_false_path -rise_from [get_clocks mac_clk_125_virtual] -fall_to [get_clocks rgmii_rxclk] -setup
set_false_path -fall_from [get_clocks mac_clk_125_virtual] -fall_to [get_clocks rgmii_rxclk] -hold
set_false_path -rise_from [get_clocks mac_clk_125_virtual] -rise_to [get_clocks rgmii_rxclk] -hold  

set_clock_groups -asynchronous -group {CLK_125M} -group {rgmii_rxclk}