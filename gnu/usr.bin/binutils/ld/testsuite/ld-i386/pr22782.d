#as: --32
#ld: -melf_i386
#readelf: -SWs

#failif
#...
[ 	]*\[.*\][ 	]+.*\.got\.plt .*
#...
.* _GLOBAL_OFFSET_TABLE_
#...
