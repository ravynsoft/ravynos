#ld: -pie -melf_i386
#readelf: -S --wide
#as: --32

#...
[ 	]*\[.*\][ 	]+.*\.got\.plt.*
#pass
