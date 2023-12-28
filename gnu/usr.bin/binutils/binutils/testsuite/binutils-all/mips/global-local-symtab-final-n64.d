#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS symbol table sort and section symbol names (fully linked) (n64)
#as: -64 -mno-pdr -mips3
#ld: -e 0 -T ../../../../ld/testsuite/ld-mips-elf/global-local-symtab.ld
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -s
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s
#dump: global-local-symtab-sort-n64t.d
