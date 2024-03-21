#source: exclude3.s
#ld: -r
#readelf: -S --wide

#...
[ 	]*\[.*\][ 	]+\.foo1[ 	]+PROGBITS.*[ 	]+E[   ]+.*
#pass
