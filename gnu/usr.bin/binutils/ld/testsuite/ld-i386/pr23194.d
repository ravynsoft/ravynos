#as: --32 -mrelax-relocations=yes
#ld: -shared -melf_i386 --version-script pr23194.map
#readelf: -r --wide

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset     Info    Type                Sym. Value  Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_GLOB_DAT +[0-9a-f]+ +foobar@@FOO
