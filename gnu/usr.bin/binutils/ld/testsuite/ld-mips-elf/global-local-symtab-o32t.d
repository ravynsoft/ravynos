#name: MIPS global/local symbol table split (o32)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -r -T global-local-symtab.ld
#readelf: -S
#...
 *\[ *[0-9]+\] +\.symtab +SYMTAB +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +10 +3 +4 +4
#pass
