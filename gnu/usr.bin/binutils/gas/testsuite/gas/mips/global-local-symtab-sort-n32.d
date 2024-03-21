#DUMPPROG: readelf
#readelf: -s
#name: MIPS global/local symbol table sort and section symbol names (n32)
#as: -n32 -mno-pdr -mips3
#source: global-local-symtab.s
#dump: global-local-symtab-sort-o32.d
