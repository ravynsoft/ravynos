#readelf: -SW
#name: automatic section group a
#source: groupauto.s
# The RX port uses non-standard section names.
#notarget: rx-*

#...
[ 	]*\[.*\][ 	]+\.group[ 	]+GROUP.*
#...
[ 	]*\[.*\][ 	]+\.text[ 	]+PROGBITS.*[ 	]+AX[ 	]+.*
#...
[ 	]*\[.*\][ 	]+\.foo[ 	]+PROGBITS.*[ 	]+A[ 	]+.*
#...
[ 	]*\[.*\][ 	]+\.text[ 	]+PROGBITS.*[ 	]+AXG[ 	]+.*
#...
[ 	]*\[.*\][ 	]+\.note.bar[ 	]+NOTE.*[ 	]+G[ 	]+.*
#pass
