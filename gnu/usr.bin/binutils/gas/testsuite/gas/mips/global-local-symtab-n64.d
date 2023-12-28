#DUMPPROG: readelf
#readelf: -SW
#name: MIPS global/local symbol table split (n64)
#as: -64 -mno-pdr -mips3
#source: global-local-symtab.s
#...
 *\[ *[0-9]+\] +\.symtab +SYMTAB +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +18 +8 +8 +8
#pass
