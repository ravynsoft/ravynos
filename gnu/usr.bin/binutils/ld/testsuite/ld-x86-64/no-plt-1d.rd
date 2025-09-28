#readelf: -Wr
#target: x86_64-*-*

#failif
#...
[0-9a-f ]+R_X86_64_GLOB_DAT +.*
#pass
