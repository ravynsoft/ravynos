#as: -mrelax-relocations=yes
#readelf: -rs
#name: i386 local PIC

Relocation section '.rel.text' at offset 0x[0-9a-f]+ contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
[0-9a-f]+ +[0-9a-f]+ R_386_GOT32X +[0-9a-f]+ +foo
[0-9a-f]+ +[0-9a-f]+ R_386_GOT32X +[0-9a-f]+ +bar
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +NOTYPE +LOCAL +DEFAULT +[0-9]+ +foo
 +[0-9]+: +fffffff0 +[0-9a-f]+ +NOTYPE +LOCAL +DEFAULT +ABS +bar
#pass
