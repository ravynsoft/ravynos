#PROG: objcopy
#source: exclude-1.s
#objcopy:
#readelf: -S --wide
#name: objcopy on sections with SHF_EXCLUDE

#...
[ 	]*\[.*\][ 	]+\.foo1[ 	]+PROGBITS.*[ 	]+E[   ]+.*
#pass
