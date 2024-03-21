#source: pr22782.s
#as: --64
#ld: -melf_x86_64
#readelf: -SWs

#failif
#...
[ 	]*\[.*\][ 	]+.*\.got\.plt .*
#...
.* _GLOBAL_OFFSET_TABLE_
#...
