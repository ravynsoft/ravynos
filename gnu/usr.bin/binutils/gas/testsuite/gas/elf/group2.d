#readelf: -SW
#name: debug info in group section and non-group section with same name
#source: group2.s

#...
[ 	]*\[.*\][ 	]+\.group[ 	]+GROUP.*
#...
[ 	]*\[.*\][ 	]+\.text\.startup[ 	]+PROGBITS.*[ 	]+AXG[ 	]+.*
[ 	]*\[.*\][ 	]+\.text\.startup[ 	]+PROGBITS.*[ 	]+AX[ 	]+.*
#pass
