#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS symbol table sort and section symbol names (fully linked) (o32)
#as: -32 -mno-pdr
#ld: -e 0 -T ../../../../ld/testsuite/ld-mips-elf/global-local-symtab.ld
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -s
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s
#dump: global-local-symtab-sort-o32t.d
