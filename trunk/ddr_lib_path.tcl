# make sure the DDR Megawizard path is on auto_path
set ddr_lib_path "C:\\altera\\70\\ip\\ddr_ddr2_sdram\\lib\\tcl"
if { [lsearch -exact $auto_path $ddr_lib_path] == -1 } {
	lappend auto_path $ddr_lib_path
}
return
