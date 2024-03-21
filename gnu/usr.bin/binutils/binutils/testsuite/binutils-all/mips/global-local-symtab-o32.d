#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS global/local symbol table split (o32)
#as: -32 -mno-pdr
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -S
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s
#...
 *\[ *[0-9]+\] +\.symtab +SYMTAB +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +10 +3 +2 +4
#pass
