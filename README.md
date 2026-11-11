# Nios V Example Designs Repository

This repository contains the Nios V Example designs based on different Altera FPGA development kits.

The following table contains the list of Acronyms that the user may come across in the design details

| Acronym | Expansion |
| --- | ------ |
| DMA | Direct Memory Access |
| OCM | On-Chip Memory |
| PIO | Parallel I/O |
| RTOS | Real Time Operating System |
| ECC | Error-Correcting Code |
| TCM | Tightly Coupled Memory |
| SSS | Simple Socket Server |
| CI | Custom Instrcution |
| CRC | Cyclic Redundancy Check |


There are three variants of the NiosV core:
    
    a. Nios V/m core - Microcontroller- Balanced (For interrupt driven baremetal and RTOS code)
    
    b. Nios V/g core - General-Purpose Processor- High Performance (For interrupt driven baremetal and RTOS code)

    c. Nios V/c core - Compact Microcontroller- Smallest (For non-interrupt driven baremetal code)

# 1. a5e065b-prem-devkit   
Example Designs using Nios V as the core based on Agilex™ 5 FPGA E-Series 065B Premium Development Kit

Development Kit product page-  https://www.altera.com/products/devkit/po-3284/agilex-5-fpga-e-series-065b-premium-development-kit.html

The following table contains the list of the designs on Agilex 5 FPGA E-Series 065B Premium Development Kit

| No # | Design Name Prefix (Nios V core) | Design Name Suffix (Functions) | Description |
| - | --- | ------ | ----------- |
| 1 | Nios V/g | Nios V/g Iperf System Example Design | This design demonstrates the Iperf application on a Nios V/g processor using the Triple Speed Ethernet IP for the Agilex 5 FPGA and SoC E-Series Premium Development Kit (ES) board. <br>[Design details](niosv_g/niosv_g_webserver_iperf/docs/NiosV_g_Processor_iperf_on_Agilex_5_FPGA.md)|
