#readelf: -Wr
#target: i?86-*-*

#failif
#...
[0-9a-f ]+R_386_GLOB_DAT +.*
#pass
