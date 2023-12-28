#source: exclude3.s
#ld:
#readelf: -S --wide

#failif
#...
[ 	]*\[.*\][ 	]+\.foo1[ 	]+.*
#...
