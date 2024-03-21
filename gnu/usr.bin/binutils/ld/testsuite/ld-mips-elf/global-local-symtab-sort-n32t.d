#name: MIPS symbol table sort and section symbol names (relocatable) (n32)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -r -T global-local-symtab.ld
#readelf: -sW
#dump: global-local-symtab-sort-o32t.d
