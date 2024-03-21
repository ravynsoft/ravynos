#name: MIPS symbol table sort and section symbol names (fully linked) (n32)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -e 0 -T global-local-symtab.ld
#readelf: -sW
#dump: global-local-symtab-sort-o32t.d
