#as: -march=rv64i_ztso
#readelf: -h
#source: empty.s

ELF Header:
#...
[ 	]+Flags:[ 	]+0x10, TSO.*
#...