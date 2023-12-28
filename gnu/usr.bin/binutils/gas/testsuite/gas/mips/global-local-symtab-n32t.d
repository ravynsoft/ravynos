#DUMPPROG: readelf
#readelf: -S
#name: MIPS global/local symbol table split (n32)
#as: -n32 -mno-pdr -mips3
#source: global-local-symtab.s
#dump: global-local-symtab-o32t.d
