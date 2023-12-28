#readelf: -Wr
#target: i?86-*-*

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains [0-9]+ entries:
 +Offset +Info +Type +Sym. Value +Symbol's Name
#...
[0-9a-f ]+R_386_GLOB_DAT +0+ +(abort|puts).*
#...
[0-9a-f ]+R_386_GLOB_DAT +0+ +(abort|puts).*
#pass
