set CABLE_INDEX 0
set DEVICE_INDEX 0
set ISSP_INDEX_0 0
set RESET_MS 100
 
set hw_name [lindex [get_hardware_names] $CABLE_INDEX]
set dev_name [lindex [get_device_names -hardware_name $hw_name] $DEVICE_INDEX]
 
start_insystem_source_probe  -hardware_name $hw_name -device_name $dev_name
 
# Create a reset pulse
#write_source_data -instance_index $ISSP_INDEX_0 -value 0
write_source_data -instance_index $ISSP_INDEX_0 -value 0x0 -value_in_hex
after $RESET_MS
write_source_data -instance_index $ISSP_INDEX_0 -value 0x3 -value_in_hex

