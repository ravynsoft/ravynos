#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS symbol table sort and section symbol names (relocatable) (n32)
#as: -n32 -mno-pdr -mips3
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -s
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s
#dump: global-local-symtab-sort-o32t.d
