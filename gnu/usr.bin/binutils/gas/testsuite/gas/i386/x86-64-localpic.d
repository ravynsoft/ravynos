#as: -mrelax-relocations=yes
#readelf: -rsW
#name: x86-64 local PIC

Relocation section '.rela.text' at offset 0x[0-9a-f]+ contains 2 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ R_X86_64_REX_GOTPCRELX +[0-9a-f]+ +foo - 4
[0-9a-f]+ +[0-9a-f]+ R_X86_64_REX_GOTPCRELX +[0-9a-f]+ +bar - 4
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +NOTYPE +LOCAL +DEFAULT +[0-9]+ +foo
 +[0-9]+: +0+fffffff0 +[0-9a-f]+ +NOTYPE +LOCAL +DEFAULT +ABS +bar
#pass
