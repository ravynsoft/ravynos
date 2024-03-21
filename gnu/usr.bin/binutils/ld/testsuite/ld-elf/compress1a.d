#source: compress1.s
#as: --compress-debug-sections
#ld: -e func_cu2
#readelf: -S --wide
#xfail: alpha-*-*ecoff

#failif
#...
  \[[ 0-9]+\] \.zdebug_.*[ 	]+(PROGBITS|MIPS_DWARF)[ 	0-9a-z]+ .*
#...
