#DUMPPROG: readelf
#readelf: -S
#name: MIPS global/local symbol table split (o32)
#as: -32 -mno-pdr
#source: global-local-symtab.s
#...
 *\[ *[0-9]+\] +\.symtab +SYMTAB +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +10 +8 +8 +4
#pass
