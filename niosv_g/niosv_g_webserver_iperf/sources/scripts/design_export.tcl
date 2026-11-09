load_package vds
# project_new top
# 
# Info: Current script was exported with Quartus Prime Pro Release 26.3.0 Build 101
# 
regexp {[\.0-9]+} $quartus(version) import_release
if {$import_release != "26.3.0"} {
	post_message -type error "Cannot import script exported from Quartus Prime Pro version \"26.3.0 Build 101\" into current release \"$import_release\".\n\tOpen, upgrade, and validate the original system in VDS to ensure design correctness."
	return
}
proc get_current_hier {} {
set current_hier [vds::get_current_hier]
if {$current_hier == "|"} {
	set current_hier ""
}
return $current_hier
}

project_open top

vds::create_system qsys_top
vds::create_cell -properties { parameters:lvl1OutputRegA {true} parameters:lvl2OutputRegA {true} parameters:memorySize {524288}  } -vlnv altera.com:ip:intel_onchip_memory: boot_mem
vds::create_cell -properties { parameters:EXPLICIT_CLOCK_RATE {155000000}  } -vlnv altera.com:ip:altera_clock_bridge: clock_bridge_0
vds::create_cell -properties { parameters:dataCacheSize {16384} parameters:enableBranchPrediction {false} parameters:instCacheSize {0} parameters:peripheralRegionABase {598016} parameters:peripheralRegionASize {8192} parameters:resetSlave {boot_mem.s1}  } -vlnv altera.com:ip:intel_niosv_g: intel_niosv_g_0
vds::create_cell -vlnv altera.com:ip:altera_reset_bridge: reset_bridge_0
vds::create_cell -properties { parameters:AXI_interface {2} parameters:blockType {M20K} parameters:dualPort {1} parameters:idWidth {4} parameters:lvl1OutputRegA {true} parameters:lvl1OutputRegB {true} parameters:lvl2OutputRegA {true} parameters:lvl2OutputRegB {true} parameters:memorySize {2097152}  } -vlnv altera.com:ip:intel_onchip_memory: sys_cpu_ram
vds::create_cell -properties { parameters:idWidth {4} parameters:memorySize {8192}  } -vlnv altera.com:ip:intel_onchip_memory: sys_desc_mem
vds::create_cell -vlnv altera.com:ip:altera_avalon_jtag_uart: sys_jtag_uart
vds::create_cell -properties { parameters:width {4}  } -vlnv altera.com:ip:altera_avalon_pio: sys_led_pio
vds::create_cell -properties { parameters:id {-87110914}  } -vlnv altera.com:ip:altera_avalon_sysid_qsys: sys_sysid
vds::create_cell -properties { parameters:core_variation {MAC_ONLY} parameters:eg_addr {13} parameters:enable_gmii_loopback {true} parameters:enable_hd_logic {false} parameters:ifGMII {RGMII} parameters:ing_addr {13} parameters:max_channels {4} parameters:mdio_clk_div {50} parameters:rx_adaptation_mode {manual} parameters:useMDIO {true} parameters:ux_txeq_main_tap {52} parameters:ux_txeq_post_tap_1 {5}  } -vlnv altera.com:ip:intel_eth_tse: sys_tse
vds::create_cell -properties { parameters:BURST_ENABLE {1} parameters:BURST_WRAPPING_SUPPORT {1} parameters:DATA_FIFO_DEPTH {4096} parameters:ERROR_ENABLE {1} parameters:ERROR_WIDTH {6} parameters:MAX_BURST_COUNT {64} parameters:MAX_BYTE {262144} parameters:MODE {2} parameters:PACKET_ENABLE {1} parameters:PREFETCHER_ENABLE {1} parameters:TRANSFER_TYPE {Unaligned Accesses}  } -vlnv altera.com:ip:altera_msgdma: sys_tse_msgdma_rx
vds::create_cell -properties { parameters:BURST_ENABLE {1} parameters:BURST_WRAPPING_SUPPORT {1} parameters:DATA_FIFO_DEPTH {4096} parameters:ERROR_ENABLE {1} parameters:ERROR_WIDTH {1} parameters:MAX_BURST_COUNT {64} parameters:MAX_BYTE {262144} parameters:MODE {1} parameters:PACKET_ENABLE {1} parameters:PREFETCHER_ENABLE {1} parameters:TRANSFER_TYPE {Unaligned Accesses}  } -vlnv altera.com:ip:altera_msgdma: sys_tse_msgdma_tx
vds::connect_interface_net -dest boot_mem|s1 -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/boot_mem.s1 0x0000
vds::connect_interface_net -dest boot_mem|s1 -src intel_niosv_g_0|instruction_manager
vds::assign_base_address -connection intel_niosv_g_0.instruction_manager/boot_mem.s1 0x0000
vds::connect_interface_net -dest intel_niosv_g_0|dm_agent -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/intel_niosv_g_0.dm_agent 0x00080000
vds::connect_interface_net -dest intel_niosv_g_0|dm_agent -src intel_niosv_g_0|instruction_manager
vds::assign_base_address -connection intel_niosv_g_0.instruction_manager/intel_niosv_g_0.dm_agent 0x00080000
vds::connect_interface_net -dest intel_niosv_g_0|timer_sw_agent -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/intel_niosv_g_0.timer_sw_agent 0x00092400
vds::connect_interface_net -dest sys_jtag_uart|avalon_jtag_slave -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_jtag_uart.avalon_jtag_slave 0x000924d8
vds::connect_interface_net -dest sys_tse|control_port -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_tse.control_port 0x00092000
vds::connect_interface_net -dest sys_sysid|control_slave -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_sysid.control_slave 0x000924d0
vds::connect_interface_net -dest sys_tse_msgdma_rx|csr -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_tse_msgdma_rx.csr 0x000924a0
vds::connect_interface_net -dest sys_tse_msgdma_tx|csr -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_tse_msgdma_tx.csr 0x00092480
vds::connect_interface_net -dest sys_tse_msgdma_rx|prefetcher_csr -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_tse_msgdma_rx.prefetcher_csr 0x00092460
vds::connect_interface_net -dest sys_tse_msgdma_tx|prefetcher_csr -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_tse_msgdma_tx.prefetcher_csr 0x00092440
vds::connect_interface_net -dest sys_led_pio|s1 -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_led_pio.s1 0x000924c0
vds::connect_interface_net -dest sys_desc_mem|s1 -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_desc_mem.s1 0x00090000
vds::connect_interface_net -dest sys_cpu_ram|s2 -src intel_niosv_g_0|data_manager
vds::assign_base_address -connection intel_niosv_g_0.data_manager/sys_cpu_ram.s2 0x00200000
vds::connect_interface_net -dest sys_tse_msgdma_rx|csr_irq -src intel_niosv_g_0|platform_irq_rx
vds::set_interrupt_irq -connection intel_niosv_g_0.platform_irq_rx/sys_tse_msgdma_rx.csr_irq 1
vds::connect_interface_net -dest sys_tse_msgdma_tx|csr_irq -src intel_niosv_g_0|platform_irq_rx
vds::set_interrupt_irq -connection intel_niosv_g_0.platform_irq_rx/sys_tse_msgdma_tx.csr_irq 2
vds::connect_interface_net -dest sys_jtag_uart|irq -src intel_niosv_g_0|platform_irq_rx
vds::set_interrupt_irq -connection intel_niosv_g_0.platform_irq_rx/sys_jtag_uart.irq 0
vds::connect_interface_net -dest sys_cpu_ram|s2 -src sys_tse_msgdma_tx|mm_read
vds::assign_base_address -connection sys_tse_msgdma_tx.mm_read/sys_cpu_ram.s2 0x00200000
vds::connect_interface_net -dest sys_cpu_ram|s1 -src sys_tse_msgdma_rx|mm_write
vds::assign_base_address -connection sys_tse_msgdma_rx.mm_write/sys_cpu_ram.s1 0x00200000
vds::connect_interface_net -dest sys_desc_mem|s1 -src sys_tse_msgdma_rx|descriptor_read_master
vds::assign_base_address -connection sys_tse_msgdma_rx.descriptor_read_master/sys_desc_mem.s1 0x00090000
vds::connect_interface_net -dest sys_desc_mem|s1 -src sys_tse_msgdma_tx|descriptor_read_master
vds::assign_base_address -connection sys_tse_msgdma_tx.descriptor_read_master/sys_desc_mem.s1 0x00090000
vds::connect_interface_net -dest sys_desc_mem|s1 -src sys_tse_msgdma_rx|descriptor_write_master
vds::assign_base_address -connection sys_tse_msgdma_rx.descriptor_write_master/sys_desc_mem.s1 0x00090000
vds::connect_interface_net -dest sys_desc_mem|s1 -src sys_tse_msgdma_tx|descriptor_write_master
vds::assign_base_address -connection sys_tse_msgdma_tx.descriptor_write_master/sys_desc_mem.s1 0x00090000
vds::connect_interface_net -dest sys_tse|transmit -src sys_tse_msgdma_tx|st_source
vds::connect_interface_net -dest sys_tse_msgdma_rx|st_sink -src sys_tse|receive
vds::export_interface_pin sys_tse|mac_misc_connection triple_speed_ethernet_0_mac_misc_connection
vds::export_interface_pin sys_tse|mac_status_connection sys_tse_mac_status_connection
vds::export_interface_pin sys_tse|mac_rgmii_connection sys_tse_mac_rgmii_connection
vds::export_interface_pin sys_tse|mac_mdio_connection sys_tse_mac_mdio_connection
vds::export_pin clock_bridge_0|in_clk clk
vds::connect_net -src clock_bridge_0|out_clk -dest reset_bridge_0|clk
vds::connect_net -src clock_bridge_0|out_clk -dest sys_jtag_uart|clk
vds::connect_net -src clock_bridge_0|out_clk -dest sys_led_pio|clk
vds::connect_net -src clock_bridge_0|out_clk -dest sys_sysid|clk
vds::connect_net -src clock_bridge_0|out_clk -dest intel_niosv_g_0|clk
vds::connect_net -src clock_bridge_0|out_clk -dest sys_tse|clk
vds::connect_net -src clock_bridge_0|out_clk -dest sys_cpu_ram|clk1
vds::connect_net -src clock_bridge_0|out_clk -dest sys_desc_mem|clk1
vds::connect_net -src clock_bridge_0|out_clk -dest boot_mem|clk1
vds::connect_net -src clock_bridge_0|out_clk -dest sys_tse_msgdma_rx|clock
vds::connect_net -src clock_bridge_0|out_clk -dest sys_tse_msgdma_tx|clock
vds::connect_net -src clock_bridge_0|out_clk -dest sys_tse|ff_rx_clk
vds::connect_net -src clock_bridge_0|out_clk -dest sys_tse|ff_tx_clk
vds::export_pin reset_bridge_0|in_reset reset
vds::connect_net -src reset_bridge_0|out_reset -dest intel_niosv_g_0|reset
vds::connect_net -src reset_bridge_0|out_reset -dest sys_jtag_uart|reset
vds::connect_net -src reset_bridge_0|out_reset -dest sys_led_pio|reset
vds::connect_net -src reset_bridge_0|out_reset -dest sys_sysid|reset
vds::connect_net -src reset_bridge_0|out_reset -dest sys_tse|reset
vds::connect_net -src reset_bridge_0|out_reset -dest sys_cpu_ram|reset1
vds::connect_net -src reset_bridge_0|out_reset -dest sys_desc_mem|reset1
vds::connect_net -src reset_bridge_0|out_reset -dest boot_mem|reset1
vds::connect_net -src reset_bridge_0|out_reset -dest sys_tse_msgdma_rx|reset_n
vds::connect_net -src reset_bridge_0|out_reset -dest sys_tse_msgdma_tx|reset_n
vds::export_pin sys_led_pio|external_connection sys_led_pio_external_connection
vds::export_pin sys_tse|rx_clk sys_tse_pcs_mac_rx_clock_connection
vds::export_pin sys_tse|tx_clk sys_tse_pcs_mac_tx_clock_connection
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter FIFO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize TRUE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType EMBEDDED_MEMORY_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } intel_niosv_g_0.data_manager
vds::set_domain_properties {qsys_mm.burstAdapterImplementation GENERIC_CONVERTER qsys_mm.clockCrossingAdapter AUTO qsys_mm.enableAllPipelines FALSE qsys_mm.enableEccProtection FALSE qsys_mm.enableInstrumentation FALSE qsys_mm.enableOutOfOrderSupport FALSE qsys_mm.fifoDepth 8 qsys_mm.insertDefaultSlave FALSE qsys_mm.interconnectResetSource DEFAULT qsys_mm.maxAdditionalLatency 1 qsys_mm.optimizeRdFifoSize FALSE qsys_mm.piplineType PIPELINE_STAGE qsys_mm.responseFifoType REGISTER_BASED qsys_mm.splitCommandsFor4KBoundary FALSE qsys_mm.syncResets TRUE qsys_mm.widthAdapterImplementation GENERIC_CONVERTER } sys_tse_msgdma_rx.mm_write
vds::set_pipelines {mm_interconnect_0|cmd_demux.src0/cmd_mux.sink0 0 mm_interconnect_0|cmd_demux.src1/cmd_mux_001.sink0 0 mm_interconnect_0|cmd_demux.src10/cmd_mux_010.sink0 0 mm_interconnect_0|cmd_demux.src11/cmd_mux_011.sink0 0 mm_interconnect_0|cmd_demux.src2/cmd_mux_002.sink0 0 mm_interconnect_0|cmd_demux.src3/cmd_mux_003.sink0 0 mm_interconnect_0|cmd_demux.src4/cmd_mux_004.sink0 0 mm_interconnect_0|cmd_demux.src5/cmd_mux_005.sink0 0 mm_interconnect_0|cmd_demux.src6/cmd_mux_006.sink0 0 mm_interconnect_0|cmd_demux.src7/cmd_mux_007.sink0 0 mm_interconnect_0|cmd_demux.src8/cmd_mux_008.sink0 0 mm_interconnect_0|cmd_demux.src9/cmd_mux_009.sink0 0 mm_interconnect_0|cmd_demux_001.src0/cmd_mux.sink1 0 mm_interconnect_0|cmd_demux_001.src1/cmd_mux_001.sink1 0 mm_interconnect_0|cmd_demux_001.src10/cmd_mux_010.sink1 0 mm_interconnect_0|cmd_demux_001.src11/cmd_mux_011.sink1 0 mm_interconnect_0|cmd_demux_001.src2/cmd_mux_002.sink1 0 mm_interconnect_0|cmd_demux_001.src3/cmd_mux_003.sink1 0 mm_interconnect_0|cmd_demux_001.src4/cmd_mux_004.sink1 0 mm_interconnect_0|cmd_demux_001.src5/cmd_mux_005.sink1 0 mm_interconnect_0|cmd_demux_001.src6/cmd_mux_006.sink1 0 mm_interconnect_0|cmd_demux_001.src7/cmd_mux_007.sink1 0 mm_interconnect_0|cmd_demux_001.src8/cmd_mux_008.sink1 0 mm_interconnect_0|cmd_demux_001.src9/cmd_mux_009.sink1 0 mm_interconnect_0|cmd_demux_002.src0/cmd_mux_008.sink2 0 mm_interconnect_0|cmd_demux_002.src1/cmd_mux_009.sink2 0 mm_interconnect_0|cmd_demux_003.src0/cmd_mux_008.sink3 0 mm_interconnect_0|cmd_demux_003.src1/cmd_mux_009.sink3 0 mm_interconnect_0|cmd_demux_004.src0/cmd_mux_008.sink4 0 mm_interconnect_0|cmd_demux_004.src1/cmd_mux_009.sink4 0 mm_interconnect_0|cmd_demux_005.src0/cmd_mux_008.sink5 0 mm_interconnect_0|cmd_demux_005.src1/cmd_mux_009.sink5 0 mm_interconnect_0|cmd_demux_006.src0/cmd_mux_008.sink6 0 mm_interconnect_0|cmd_demux_006.src1/cmd_mux_009.sink6 0 mm_interconnect_0|cmd_demux_007.src0/cmd_mux_008.sink7 0 mm_interconnect_0|cmd_demux_007.src1/cmd_mux_009.sink7 0 mm_interconnect_0|cmd_demux_008.src0/cmd_mux_005.sink2 0 mm_interconnect_0|cmd_demux_008.src1/cmd_mux_008.sink8 0 mm_interconnect_0|cmd_demux_009.src0/cmd_mux_005.sink3 0 mm_interconnect_0|cmd_demux_009.src1/cmd_mux_008.sink9 0 mm_interconnect_0|cmd_mux 0 mm_interconnect_0|cmd_mux.src/sys_jtag_uart_avalon_jtag_slave_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_001 0 mm_interconnect_0|cmd_mux_001.src/sys_tse_control_port_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_002 0 mm_interconnect_0|cmd_mux_002.src/sys_sysid_control_slave_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_003 0 mm_interconnect_0|cmd_mux_003.src/sys_tse_msgdma_rx_csr_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_004 0 mm_interconnect_0|cmd_mux_004.src/sys_tse_msgdma_tx_csr_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_005 0 mm_interconnect_0|cmd_mux_005.src/intel_niosv_g_0_dm_agent_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_006 0 mm_interconnect_0|cmd_mux_006.src/sys_tse_msgdma_rx_prefetcher_csr_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_007 0 mm_interconnect_0|cmd_mux_007.src/sys_tse_msgdma_tx_prefetcher_csr_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_008 0 mm_interconnect_0|cmd_mux_008.src/sys_cpu_ram_s1_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_009 0 mm_interconnect_0|cmd_mux_009.src/sys_desc_mem_s1_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_010 0 mm_interconnect_0|cmd_mux_010.src/sys_led_pio_s1_burst_adapter.sink0 0 mm_interconnect_0|cmd_mux_011 0 mm_interconnect_0|cmd_mux_011.src/intel_niosv_g_0_timer_sw_agent_burst_adapter.sink0 0 mm_interconnect_0|intel_niosv_g_0_data_manager_agent.read_cp/router_001.sink 1 mm_interconnect_0|intel_niosv_g_0_data_manager_agent.write_cp/router.sink 1 mm_interconnect_0|intel_niosv_g_0_dm_agent_agent.rp/router_015.sink 0 mm_interconnect_0|intel_niosv_g_0_dm_agent_burst_adapter.source0/intel_niosv_g_0_dm_agent_agent.cp 0 mm_interconnect_0|intel_niosv_g_0_instruction_manager_agent.read_cp/router_009.sink 0 mm_interconnect_0|intel_niosv_g_0_instruction_manager_agent.write_cp/router_008.sink 0 mm_interconnect_0|intel_niosv_g_0_instruction_manager_rd_limiter.cmd_src/cmd_demux_009.sink 0 mm_interconnect_0|intel_niosv_g_0_instruction_manager_rd_limiter.rsp_src/intel_niosv_g_0_instruction_manager_agent.read_rp 1 mm_interconnect_0|intel_niosv_g_0_instruction_manager_wr_limiter.cmd_src/cmd_demux_008.sink 0 mm_interconnect_0|intel_niosv_g_0_instruction_manager_wr_limiter.rsp_src/intel_niosv_g_0_instruction_manager_agent.write_rp 1 mm_interconnect_0|intel_niosv_g_0_timer_sw_agent_agent.rp/router_021.sink 0 mm_interconnect_0|intel_niosv_g_0_timer_sw_agent_burst_adapter.source0/intel_niosv_g_0_timer_sw_agent_agent.cp 0 mm_interconnect_0|router.src/cmd_demux.sink 0 mm_interconnect_0|router_001.src/cmd_demux_001.sink 0 mm_interconnect_0|router_002.src/sys_tse_msgdma_rx_descriptor_read_master_limiter.cmd_sink 0 mm_interconnect_0|router_003.src/sys_tse_msgdma_tx_descriptor_read_master_limiter.cmd_sink 0 mm_interconnect_0|router_004.src/cmd_demux_004.sink 0 mm_interconnect_0|router_005.src/cmd_demux_005.sink 0 mm_interconnect_0|router_006.src/sys_tse_msgdma_tx_mm_read_limiter.cmd_sink 0 mm_interconnect_0|router_007.src/cmd_demux_007.sink 0 mm_interconnect_0|router_008.src/intel_niosv_g_0_instruction_manager_wr_limiter.cmd_sink 0 mm_interconnect_0|router_009.src/intel_niosv_g_0_instruction_manager_rd_limiter.cmd_sink 0 mm_interconnect_0|router_010.src/rsp_demux.sink 0 mm_interconnect_0|router_011.src/rsp_demux_001.sink 1 mm_interconnect_0|router_012.src/rsp_demux_002.sink 0 mm_interconnect_0|router_013.src/rsp_demux_003.sink 1 mm_interconnect_0|router_014.src/rsp_demux_004.sink 1 mm_interconnect_0|router_015.src/rsp_demux_005.sink 0 mm_interconnect_0|router_016.src/rsp_demux_006.sink 1 mm_interconnect_0|router_017.src/rsp_demux_007.sink 1 mm_interconnect_0|router_018.src/rsp_demux_008.sink 0 mm_interconnect_0|router_019.src/rsp_demux_009.sink 0 mm_interconnect_0|router_020.src/rsp_demux_010.sink 0 mm_interconnect_0|router_021.src/rsp_demux_011.sink 0 mm_interconnect_0|rsp_demux.src0/rsp_mux.sink0 0 mm_interconnect_0|rsp_demux.src1/rsp_mux_001.sink0 0 mm_interconnect_0|rsp_demux_001.src0/rsp_mux.sink1 0 mm_interconnect_0|rsp_demux_001.src1/rsp_mux_001.sink1 0 mm_interconnect_0|rsp_demux_002.src0/rsp_mux.sink2 0 mm_interconnect_0|rsp_demux_002.src1/rsp_mux_001.sink2 0 mm_interconnect_0|rsp_demux_003.src0/rsp_mux.sink3 0 mm_interconnect_0|rsp_demux_003.src1/rsp_mux_001.sink3 0 mm_interconnect_0|rsp_demux_004.src0/rsp_mux.sink4 0 mm_interconnect_0|rsp_demux_004.src1/rsp_mux_001.sink4 0 mm_interconnect_0|rsp_demux_005.src0/rsp_mux.sink5 0 mm_interconnect_0|rsp_demux_005.src1/rsp_mux_001.sink5 0 mm_interconnect_0|rsp_demux_005.src2/rsp_mux_008.sink0 0 mm_interconnect_0|rsp_demux_005.src3/rsp_mux_009.sink0 0 mm_interconnect_0|rsp_demux_006.src0/rsp_mux.sink6 0 mm_interconnect_0|rsp_demux_006.src1/rsp_mux_001.sink6 0 mm_interconnect_0|rsp_demux_007.src0/rsp_mux.sink7 0 mm_interconnect_0|rsp_demux_007.src1/rsp_mux_001.sink7 0 mm_interconnect_0|rsp_demux_008.src0/rsp_mux.sink8 0 mm_interconnect_0|rsp_demux_008.src1/rsp_mux_001.sink8 0 mm_interconnect_0|rsp_demux_008.src2/rsp_mux_002.sink0 0 mm_interconnect_0|rsp_demux_008.src3/rsp_mux_003.sink0 0 mm_interconnect_0|rsp_demux_008.src4/rsp_mux_004.sink0 0 mm_interconnect_0|rsp_demux_008.src5/rsp_mux_005.sink0 0 mm_interconnect_0|rsp_demux_008.src6/rsp_mux_006.sink0 0 mm_interconnect_0|rsp_demux_008.src7/rsp_mux_007.sink0 0 mm_interconnect_0|rsp_demux_008.src8/rsp_mux_008.sink1 0 mm_interconnect_0|rsp_demux_008.src9/rsp_mux_009.sink1 0 mm_interconnect_0|rsp_demux_009.src0/rsp_mux.sink9 0 mm_interconnect_0|rsp_demux_009.src1/rsp_mux_001.sink9 0 mm_interconnect_0|rsp_demux_009.src2/rsp_mux_002.sink1 0 mm_interconnect_0|rsp_demux_009.src3/rsp_mux_003.sink1 0 mm_interconnect_0|rsp_demux_009.src4/rsp_mux_004.sink1 0 mm_interconnect_0|rsp_demux_009.src5/rsp_mux_005.sink1 0 mm_interconnect_0|rsp_demux_009.src6/rsp_mux_006.sink1 0 mm_interconnect_0|rsp_demux_009.src7/rsp_mux_007.sink1 0 mm_interconnect_0|rsp_demux_010.src0/rsp_mux.sink10 0 mm_interconnect_0|rsp_demux_010.src1/rsp_mux_001.sink10 0 mm_interconnect_0|rsp_demux_011.src0/rsp_mux.sink11 0 mm_interconnect_0|rsp_demux_011.src1/rsp_mux_001.sink11 0 mm_interconnect_0|rsp_mux.src/intel_niosv_g_0_data_manager_agent.write_rp 1 mm_interconnect_0|rsp_mux_001.src/intel_niosv_g_0_data_manager_agent.read_rp 1 mm_interconnect_0|rsp_mux_002.src/sys_tse_msgdma_rx_descriptor_read_master_limiter.rsp_sink 1 mm_interconnect_0|rsp_mux_003.src/sys_tse_msgdma_tx_descriptor_read_master_limiter.rsp_sink 1 mm_interconnect_0|rsp_mux_004.src/sys_tse_msgdma_rx_descriptor_write_master_agent.rp 1 mm_interconnect_0|rsp_mux_005.src/sys_tse_msgdma_tx_descriptor_write_master_agent.rp 0 mm_interconnect_0|rsp_mux_006.src/sys_tse_msgdma_tx_mm_read_limiter.rsp_sink 0 mm_interconnect_0|rsp_mux_007.src/sys_tse_msgdma_rx_mm_write_agent.rp 0 mm_interconnect_0|rsp_mux_008.src/intel_niosv_g_0_instruction_manager_wr_limiter.rsp_sink 1 mm_interconnect_0|rsp_mux_009.src/intel_niosv_g_0_instruction_manager_rd_limiter.rsp_sink 1 mm_interconnect_0|sys_cpu_ram_s1_agent.rp/router_018.sink 0 mm_interconnect_0|sys_cpu_ram_s1_burst_adapter.source0/sys_cpu_ram_s1_agent.cp 0 mm_interconnect_0|sys_desc_mem_s1_agent.rp/router_019.sink 0 mm_interconnect_0|sys_desc_mem_s1_burst_adapter.source0/sys_desc_mem_s1_agent.cp 0 mm_interconnect_0|sys_jtag_uart_avalon_jtag_slave_agent.rp/router_010.sink 0 mm_interconnect_0|sys_jtag_uart_avalon_jtag_slave_burst_adapter.source0/sys_jtag_uart_avalon_jtag_slave_agent.cp 0 mm_interconnect_0|sys_led_pio_s1_agent.rp/router_020.sink 0 mm_interconnect_0|sys_led_pio_s1_burst_adapter.source0/sys_led_pio_s1_agent.cp 0 mm_interconnect_0|sys_sysid_control_slave_agent.rp/router_012.sink 0 mm_interconnect_0|sys_sysid_control_slave_burst_adapter.source0/sys_sysid_control_slave_agent.cp 0 mm_interconnect_0|sys_tse_control_port_agent.rp/router_011.sink 1 mm_interconnect_0|sys_tse_control_port_burst_adapter.source0/sys_tse_control_port_agent.cp 0 mm_interconnect_0|sys_tse_msgdma_rx_csr_agent.rp/router_013.sink 1 mm_interconnect_0|sys_tse_msgdma_rx_csr_burst_adapter.source0/sys_tse_msgdma_rx_csr_agent.cp 0 mm_interconnect_0|sys_tse_msgdma_rx_descriptor_read_master_agent.cp/router_002.sink 1 mm_interconnect_0|sys_tse_msgdma_rx_descriptor_read_master_limiter.cmd_src/cmd_demux_002.sink 0 mm_interconnect_0|sys_tse_msgdma_rx_descriptor_read_master_limiter.rsp_src/sys_tse_msgdma_rx_descriptor_read_master_agent.rp 1 mm_interconnect_0|sys_tse_msgdma_rx_descriptor_write_master_agent.cp/router_004.sink 1 mm_interconnect_0|sys_tse_msgdma_rx_mm_write_agent.cp/router_007.sink 0 mm_interconnect_0|sys_tse_msgdma_rx_prefetcher_csr_agent.rp/router_016.sink 1 mm_interconnect_0|sys_tse_msgdma_rx_prefetcher_csr_burst_adapter.source0/sys_tse_msgdma_rx_prefetcher_csr_agent.cp 0 mm_interconnect_0|sys_tse_msgdma_tx_csr_agent.rp/router_014.sink 1 mm_interconnect_0|sys_tse_msgdma_tx_csr_burst_adapter.source0/sys_tse_msgdma_tx_csr_agent.cp 0 mm_interconnect_0|sys_tse_msgdma_tx_descriptor_read_master_agent.cp/router_003.sink 1 mm_interconnect_0|sys_tse_msgdma_tx_descriptor_read_master_limiter.cmd_src/cmd_demux_003.sink 0 mm_interconnect_0|sys_tse_msgdma_tx_descriptor_read_master_limiter.rsp_src/sys_tse_msgdma_tx_descriptor_read_master_agent.rp 1 mm_interconnect_0|sys_tse_msgdma_tx_descriptor_write_master_agent.cp/router_005.sink 1 mm_interconnect_0|sys_tse_msgdma_tx_mm_read_agent.cp/router_006.sink 1 mm_interconnect_0|sys_tse_msgdma_tx_mm_read_limiter.cmd_src/cmd_demux_006.sink 0 mm_interconnect_0|sys_tse_msgdma_tx_mm_read_limiter.rsp_src/sys_tse_msgdma_tx_mm_read_agent.rp 0 mm_interconnect_0|sys_tse_msgdma_tx_prefetcher_csr_agent.rp/router_017.sink 1 mm_interconnect_0|sys_tse_msgdma_tx_prefetcher_csr_burst_adapter.source0/sys_tse_msgdma_tx_prefetcher_csr_agent.cp 0 }
vds::validate_system
vds::save_system qsys_top

post_message "Script completed successfully."
