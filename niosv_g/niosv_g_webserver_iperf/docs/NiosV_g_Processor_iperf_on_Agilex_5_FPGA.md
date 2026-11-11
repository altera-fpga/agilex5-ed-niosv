# Nios V/g Iperf System Example Design
 
This design demonstrates the Iperf application on a Nios V/g processor using the Triple Speed Ethernet IP for the Agilex 5 FPGA and SoC E-Series Premium Development Kit (ES) board.
 
## Description
 
The System Example Design demonstrates iperf application. The Nios V/g acts as the core. The Triple Speed Ethernet (TSE) IP is configured in RGMII mode and connectes to the onboard Marvell 88E1512 PHY via RGMII interface.
 
The design has 2 MSGDMA IPs configured in Memory Mapped to Stream (MM2S) mode for Transmission and Stream to Memory Mapped (S2MM) mode for Reception.
 
To test the application, connect the RGMII Interface of the Agilex 5  FPGA board to the Link Partner using RJ-45 cable.
 
Ensure that the IP addresses are modified accordingly in the application code under the following location - sw/app_freertos/main.c
 
Once the application binaries are downloaded (See section 3.d below for the steps), start the iperf client on the link partner and observe the iperf server logs on the JTAG terminal of the board.
 
This design is created on the Quartus Visual Designer Studio. Fore more Information please visit here [Visual Designer Studio](https://www.altera.com/products/development-tools/visual-designer-studio).
 
 
![image](https://github.com/altera-fpga/agilex5-ed-niosv/blob/rel/26.3/niosv_g/niosv_g_webserver_iperf/img/web_server_block_diagram.png)
 
 
## Link Partner settings
 
1. Set the following setting on the interface of the link partner
 
    Speed- 1000 Mbps
 
    Duplex- Full
 
    Auto-Negotiation- On
 
command: ethtool -s <interface_name> speed 1000 duplex full autoneg on
 
 
## Expected Output
 
The snapshot below captures the output on the board when the elf file is downloaded (See section 3.d below for the steps to download).
 
Board IP address: 10.0.0.1
 
Link partner (linux host) IP address: 10.0.0.2
 
Speed: 1000Mbps

Command to run iperf client :  

```
iperf -M 1460 -c 10.0.0.1 -t 10 -p 5001 -w 64M -b 2500M -i 1
```

With Release-Flag 

![image](https://github.com/altera-fpga/agilex5-ed-niosv/blob/rel/26.3/niosv_g/niosv_g_webserver_iperf/img/release_expected_output.png)

With Debug-Flag

![image](https://github.com/altera-fpga/agilex5-ed-niosv/blob/rel/26.3/niosv_g/niosv_g_webserver_iperf/img/debug_expected_output.png)

Note : Any modification to the Design, Software stack or Infra will lead to a difference in the expected throughput.
 

## Project Details
 
- **Title**: Nios V/g Iperf System Example Design
- **Source**: Github
- **Design Support**: CTH
- **Family**: Agilex 5
- **Quartus Version**: 26.3
- **Development Kit**: Agilex 5 FPGA E-Series 065B Premium Development Kit DK-A5E065BB32AES1
- **Device Part**: A5ED065BB32AE6SR0
- **Design Package**: agilex5_niosv_g_webserver_iperf.zip
- **Category**: Networking
- **URL**: https://github.com/altera-fpga/agilex5-ed-niosv/blob/rel/26.3/niosv_g/niosv_g_webserver_iperf
- **download URL**: https://github.com/altera-fpga/agilex5-ed-niosv/releases/download/26.3/agilex5_niosv_g_webserver_iperf.zip
 
## Documentation
 
- **Title**: Design Document 
**URL**: https://github.com/altera-fpga/agilex5-ed-niosv/blob/rel/26.3/niosv_g/niosv_g_webserver_iperf/docs/NiosV_g_Processor_iperf_on_Agilex_5_FPGA.md
 
 
# Getting Started
 
Vendor: Altera
1. Directory structure
2. Using existing files (sof and elf) to run on hardware
3. Building the design from scratch
 
    a.	Required directory structure
 
    b.	Use of build_sof.py to compile the design
 
    c.	Steps to create the bsp and build software sources
 
    d.  Hardware Validation
 
 
### 1. Directory Structure:
 
The directory structure is explained below:
 
- hw- necessary hardware files (.qpf, .qsf, .sv, .v, .ip) of the design
 
- sw- This folder contains software application files
 
- scripts- This folder consists of scripts to build the design
 
 
### 2. Using existing files to run the design on hardware
 
- The sof and elf files required to run the design can be found in "ready_to_test" folder
 
- Refer the Hardware validation section (3.d) for the steps  
 
 
### 3. Building the design from scratch
 
The steps to build the project from scratch are mentioned below:
 
a. Required directory structure
- The top-level project folder should have directory structure as mentioned in Section 1 (Directory Structure).
 
b. Using build_sof.py to compile the design
- Invoke the quartus_py shell in the terminal
 
- Run the following command in the terminal from top level project directory:
```
cd sources
quartus_py ./scripts/build_sof.py
```
- The quartus tool will compile the design and generate the output files
 
c. Creating the bsp, build software sources and download elf
- To create software app, run the following commands in the terminal:

- Choose either Debug or Release Flag when Building the Software. 
   
- Clean the app build project before regenerating elf.

- Make sure you are inside the Nios V shell before running the following commands.
 
```     
niosv-shell
niosv-bsp --create --system=./hw/src/vds/qsys_top/qsys_top.vds --quartus-project=hw/top.qpf --type=freertos --cmd="enable_sw_package altera_freertos_tcpip" sw/bsp_freertos/settings.bsp
niosv-app --bsp-dir=sw/bsp_freertos --app-dir=sw/app_freertos --srcs=sw/app_freertos/main.c,sw/app_freertos/msgdma_driver.c,sw/app_freertos/tse_driver.c,sw/app_freertos/NetworkInterface.c,sw/app_freertos/msgdma_driver.h,sw/app_freertos/tse_driver.h
```
 
For Debug
```
cmake -S ./sw/app_freertos -B sw/app_freertos/build -DCMAKE_BUILD_TYPE=Debug
```
For Release
```
cmake -S ./sw/app_freertos -B sw/app_freertos/build -DCMAKE_BUILD_TYPE=Release
```
 
```
make -C sw/app_freertos/build
```
Note: The software can be compiled using the Ashling Visual Studio Code Extension for Altera FPGAs
 
For information on the build process, please refer to the following document- [Ashling VSCode Extension](https://www.intel.com/content/www/us/en/docs/programmable/730783/current/ashling-visual-studio-code-extension.html)
 
d. Hardware Validation
- Follow the steps mentioned below to program the sof and then download the elf file on the board
```        
quartus_pgm --cable=1 -m jtag -o 'p;ready_to_test/top.sof'
``` 
- Reduce the JTAG clock frequency to 6MHz before programming the application .elf file on the board.
```
jtagconfig --setparam 1 JtagClock 6M
```
- Toggle the In-System-Sources and Probe (ISSP) IP to initialize PHY and set it to 1000M.
```
quartus_stp -t ready_to_test/toggle_issp.tcl
```
- Download the elf file on the board 
```    
niosv-download -g -r ready_to_test/release/app_freertos.elf -c 1
``` 
- Verify the output on the terminal by using the following command in the terminal:
``` 
juart-terminal -d 1 -c 1 -i 0 
```

## Ping Application

If you want to test Ping application, please modify the following line in sw/app_freertos/main.c 

```
#define PING_TASK_PORTING_STATUS             0     // 1 - Enable, 0 - Disable 
``` 

Once the code is modified, run the make command as mentioned under section 3.c of Getting Started and follow the programming procedure mentioned under section 3.d of Getting Started.
 

