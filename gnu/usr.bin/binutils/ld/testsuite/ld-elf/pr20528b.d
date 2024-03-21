#source: pr20528b.s
#source: pr20528a.s
#ld: -r
#readelf: -S --wide
#xfail: [uses_genelf]

#...
[ 	]*\[.*\][ 	]+\.text.startup[ 	]+PROGBITS.*[ 	]+AXE[   ]+.*
#...
[ 	]*\[.*\][ 	]+\.text.startup[ 	]+PROGBITS.*[ 	]+AX[   ]+.*
#pass
