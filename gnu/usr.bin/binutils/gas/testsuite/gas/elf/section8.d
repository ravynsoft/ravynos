#readelf: -S --wide
#name: section flags

#...
[ 	]*\[.*\][ 	]+\.foo1[ 	]+PROGBITS.*[ 	]+E[   ]+.*
#pass
