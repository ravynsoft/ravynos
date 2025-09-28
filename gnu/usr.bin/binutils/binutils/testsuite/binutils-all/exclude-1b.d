#PROG: strip
#source: exclude-1.s
#strip: --strip-unneeded
#readelf: -S --wide
#name: strip --strip-unneeded on sections with SHF_EXCLUDE

#...
[ 	]*\[.*\][ 	]+\.foo1[ 	]+PROGBITS.*[ 	]+E[   ]+.*
#pass
