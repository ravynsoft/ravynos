#ld: -pie -melf_x86_64
#readelf: -S --wide
#as: --64

#...
[ 	]*\[.*\][ 	]+.*\.got\.plt.*
#pass
