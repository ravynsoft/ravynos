#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS global/local symbol table split (n64)
#as: -64 -mno-pdr -mips3
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -SW
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s
#...
 *\[ *[0-9]+\] +\.symtab +SYMTAB +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +18 +3 +3 +8
#pass
