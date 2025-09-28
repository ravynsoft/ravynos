#name: MIPS global/local symbol table split (n32)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -r -T global-local-symtab.ld
#readelf: -S
#dump: global-local-symtab-o32.d
