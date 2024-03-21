#source: pr20253-1.s
#as: --64
#ld: -pie -melf_x86_64
#readelf: -r --wide

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 2 entries:
 +Offset +Info +Type +Sym.* Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_IRELATIVE +[0-9a-f]+
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_IRELATIVE +[0-9a-f]+
