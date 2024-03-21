#source: undefweak.s
#as: --32
#ld: -shared -melf_i386 -z notext
#readelf: -r --wide

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 2 entries:
 Offset     Info    Type                Sym. Value  Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_RELATIVE +
[0-9a-f]+ +[0-9a-f]+ +R_386_32 +[0-9a-f]+ +func
