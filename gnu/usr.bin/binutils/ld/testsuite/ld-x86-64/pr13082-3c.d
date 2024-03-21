#source: pr13082-3.s
#name: PR ld/13082-3 (c)
#as: --64
#ld: -shared -melf_x86_64
#readelf: -r --wide

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
    Offset             Info             Type               Symbol's Value  Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_64 +[0-9a-f]+ +func \+ 0
