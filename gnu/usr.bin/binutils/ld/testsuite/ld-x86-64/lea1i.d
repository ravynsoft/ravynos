#source: lea1.s
#as: --64
#ld: -melf_x86_64
#readelf: -SW

#failif
#...
[ 	]*\[.*\][ 	]+.*\.got .*
#...
