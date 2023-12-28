#name: MIPS global/local symbol table split (n64)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -r -T global-local-symtab.ld
#readelf: -SW
#...
 *\[ *[0-9]+\] +\.symtab +SYMTAB +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +18 +3 +4 +8
#pass
