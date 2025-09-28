#source: compress1.s
#as: --compress-debug-sections=none
#ld: -shared --compress-debug-sections=zlib-gnu
#readelf: -SW
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#failif
#...
  \[[ 0-9]+\] \.zdebug_aranges[ 	]+(PROGBITS|MIPS_DWARF)[ 	0-9a-z]+ .*
#...
