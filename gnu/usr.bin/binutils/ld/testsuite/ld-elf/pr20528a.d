#source: pr20528a.s
#source: pr20528b.s
#ld: -r
#readelf: -S --wide
#xfail: [uses_genelf]

#...
[ 	]*\[.*\][ 	]+\.text.startup[ 	]+PROGBITS.*[ 	]+AX[   ]+.*
#...
[ 	]*\[.*\][ 	]+\.text.startup[ 	]+PROGBITS.*[ 	]+AXE[   ]+.*
#pass
