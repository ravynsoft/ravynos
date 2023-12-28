#source: undefweak.s
#as: --32
#ld: -pie -melf_i386 -z notext
#readelf: -r --wide -x .data.rel.ro

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset     Info    Type                Sym. Value  Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_RELATIVE +

Hex dump of section '.data.rel.ro':
  0x[a-f0-9]+ 00000000                            ....
