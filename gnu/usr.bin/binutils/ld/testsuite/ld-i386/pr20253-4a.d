#source: pr20253-4.s
#as: --32 -mrelax-relocations=yes
#ld: -melf_i386
#readelf: -r --wide

Relocation section '.rel.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym.* Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_IRELATIVE +
