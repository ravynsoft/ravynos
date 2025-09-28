#readelf: -SW
#name: group section (using readelf -SW)
#source: group0.s

#...
[ 	]*\[.*\][ 	]+\.group[ 	]+GROUP.*
#...
[ 	]*\[.*\][ 	]+\.foo[ 	]+PROGBITS.*[ 	]+AXG[ 	]+.*
[ 	]*\[.*\][ 	]+\.bar[ 	]+PROGBITS.*[ 	]+AG[ 	]+.*
#pass
