#as: --64 -mrelax-relocations=yes
#ld: -shared -melf_x86_64 --version-script pr23194.map
#readelf: -r --wide

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
    Offset             Info             Type               Symbol's Value  Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +[0-9a-f]+ +foobar@@FOO \+ 0
