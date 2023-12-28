#source: pr22782.s
#as: --x32
#ld: -melf32_x86_64
#readelf: -SWs

#failif
#...
[ 	]*\[.*\][ 	]+.*\.got\.plt .*
#...
.* _GLOBAL_OFFSET_TABLE_
#...
