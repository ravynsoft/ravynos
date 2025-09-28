#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS global/local symbol table split (n32)
#as: -n32 -mno-pdr -mips3
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -S
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s
#dump: global-local-symtab-o32.d
