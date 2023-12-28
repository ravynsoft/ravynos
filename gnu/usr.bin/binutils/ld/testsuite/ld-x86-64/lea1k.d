#source: lea1.s
#as: --x32
#ld: -pie -melf32_x86_64
#readelf: -SW

#failif
#...
[ 	]*\[.*\][ 	]+.*\.got .*
#...
