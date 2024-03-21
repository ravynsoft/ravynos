#readelf: -SW
#name: group section with multiple sections of same name (using readelf -SW)
#source: group1.s
# The RX port uses non-standard section names.
#xfail: rx-*

#...
[ 	]*\[.*\][ 	]+\.group[ 	]+GROUP.*
#...
[ 	]*\[.*\][ 	]+\.text[ 	]+PROGBITS.*[ 	]+AX[ 	]+.*
#...
[ 	]*\[.*\][ 	]+\.text[ 	]+PROGBITS.*[ 	]+AXG[ 	]+.*
#pass
