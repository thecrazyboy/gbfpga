# TCL File Generated by Component Editor 11.0sp1
# Sun Jul 08 10:08:50 EDT 2012
# DO NOT MODIFY


# +-----------------------------------
# | 
# | SEG7_LUT_4 "SEG7 LUT x 4" v1.0
# | Altera 2012.07.08.10:08:50
# | 7 segments display LUT with 4 digits
# | 
# | D:/dev/gbfpga/SEG7_LUT_4.v
# | 
# |    ./SEG7_LUT.v syn, sim
# |    ./SEG7_LUT_4.v syn, sim
# | 
# +-----------------------------------

# +-----------------------------------
# | request TCL package from ACDS 11.0
# | 
package require -exact sopc 11.0
# | 
# +-----------------------------------

# +-----------------------------------
# | module SEG7_LUT_4
# | 
set_module_property DESCRIPTION "7 segments display LUT with 4 digits"
set_module_property NAME SEG7_LUT_4
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property GROUP Display
set_module_property AUTHOR Altera
set_module_property DISPLAY_NAME "SEG7 LUT x 4"
set_module_property TOP_LEVEL_HDL_FILE SEG7_LUT_4.v
set_module_property TOP_LEVEL_HDL_MODULE SEG7_LUT_4
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL TRUE
set_module_property STATIC_TOP_LEVEL_MODULE_NAME SEG7_LUT_4
set_module_property FIX_110_VIP_PATH false
# | 
# +-----------------------------------

# +-----------------------------------
# | files
# | 
add_file SEG7_LUT.v {SYNTHESIS SIMULATION}
add_file SEG7_LUT_4.v {SYNTHESIS SIMULATION}
# | 
# +-----------------------------------

# +-----------------------------------
# | parameters
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | display items
# | 
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point avalon_slave_0
# | 
add_interface avalon_slave_0 avalon end
set_interface_property avalon_slave_0 addressUnits WORDS
set_interface_property avalon_slave_0 associatedClock clock_sink
set_interface_property avalon_slave_0 associatedReset reset_sink
set_interface_property avalon_slave_0 bitsPerSymbol 8
set_interface_property avalon_slave_0 burstOnBurstBoundariesOnly false
set_interface_property avalon_slave_0 burstcountUnits WORDS
set_interface_property avalon_slave_0 explicitAddressSpan 0
set_interface_property avalon_slave_0 holdTime 0
set_interface_property avalon_slave_0 linewrapBursts false
set_interface_property avalon_slave_0 maximumPendingReadTransactions 0
set_interface_property avalon_slave_0 readLatency 0
set_interface_property avalon_slave_0 readWaitTime 1
set_interface_property avalon_slave_0 setupTime 0
set_interface_property avalon_slave_0 timingUnits Cycles
set_interface_property avalon_slave_0 writeWaitTime 0

set_interface_property avalon_slave_0 ENABLED true

add_interface_port avalon_slave_0 iDIG writedata Input 16
add_interface_port avalon_slave_0 iWR write Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point clock_sink
# | 
add_interface clock_sink clock end
set_interface_property clock_sink clockRate 0

set_interface_property clock_sink ENABLED true

add_interface_port clock_sink iCLK clk Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point reset_sink
# | 
add_interface reset_sink reset end
set_interface_property reset_sink associatedClock clock_sink
set_interface_property reset_sink synchronousEdges DEASSERT

set_interface_property reset_sink ENABLED true

add_interface_port reset_sink iRST_N reset_n Input 1
# | 
# +-----------------------------------

# +-----------------------------------
# | connection point conduit_end
# | 
add_interface conduit_end conduit end

set_interface_property conduit_end ENABLED true

add_interface_port conduit_end oSEG3 export Output 7
add_interface_port conduit_end oSEG2 export Output 7
add_interface_port conduit_end oSEG0 export Output 7
add_interface_port conduit_end oSEG1 export Output 7
# | 
# +-----------------------------------
