#source: pr23324.s
#as: --64 -mrelax-relocations=yes
#ld: -q -melf_x86_64 -pie
#readelf: -r --wide

Relocation section '.rela.text' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym.* Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_PC32 +[0-9a-f]+ +_start - 4
