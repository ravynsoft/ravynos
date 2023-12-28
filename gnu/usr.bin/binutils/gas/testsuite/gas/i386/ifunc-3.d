#readelf: --relocs --syms -x .text.1 -x .text.2
#name: i386 ifunc 3
#as: --generate-missing-build-notes=no

Relocation section '\.rel\.text\.1' at offset .* contains .* entries:
 Offset     Info    Type            Sym.Value  Sym. Name
00000000  ........ R_386_PC32        bar1\(\)     bar1
00000004  ........ R_386_PC32        bar2\(\)     bar2
00000008  ........ R_386_PC32        bar1\(\)     bar1
0000000c  ........ R_386_PC32        bar2\(\)     bar2
00000010  ........ R_386_32          bar1\(\)     bar1
00000018  ........ R_386_PC32        bar1\(\)     bar1
0000001c  ........ R_386_PC32        bar2\(\)     bar2
00000020  ........ R_386_PC32        bar1\(\)     bar1
00000024  ........ R_386_PC32        bar2\(\)     bar2
00000028  ........ R_386_32          bar1\(\)     bar1
0000002c  ........ R_386_PC32        abs1\(\)     abs1
00000030  ........ R_386_PC32        abs1\(\)     abs1
00000034  ........ R_386_32          abs1\(\)     abs1
00000038  ........ R_386_PC32        abs1\(\)     abs1
0000003c  ........ R_386_PC32        abs1\(\)     abs1
00000040  ........ R_386_32          abs1\(\)     abs1

Relocation section '\.rel\.text\.2' at offset .* contains .* entries:
 Offset     Info    Type            Sym.Value  Sym. Name
00000000  ........ R_386_PC32        bar1\(\)     bar1
00000004  ........ R_386_PC32        bar2\(\)     bar2
00000008  ........ R_386_PC32        bar1\(\)     bar1
0000000c  ........ R_386_PC32        bar2\(\)     bar2
00000010  ........ R_386_32          bar2\(\)     bar2
00000018  ........ R_386_PC32        bar1\(\)     bar1
0000001c  ........ R_386_PC32        bar2\(\)     bar2
00000020  ........ R_386_PC32        bar1\(\)     bar1
00000024  ........ R_386_PC32        bar2\(\)     bar2
00000028  ........ R_386_32          bar2\(\)     bar2

Symbol table '.symtab' contains .* entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
.*: 00000014     1 IFUNC   LOCAL  DEFAULT   .* bar1
.*: 00000014     1 IFUNC   LOCAL  DEFAULT   .* bar2
#...
.*: 11223300     0 IFUNC   LOCAL  DEFAULT  ABS abs1
#...

Hex dump of section '\.text\.1':
 NOTE: This section has relocations against it, but these have NOT been applied to this dump\.
  0x00000000 00000000 00000000 08000000 0c000000 .*
  0x00000010 4054ffff c38d7600 00000000 00000000 .*
  0x00000020 20000000 24000000 4054ffff 00000000 .*
  0x00000030 30000000 4054ffff 00000000 3c000000 .*
  0x00000040 4054ffff                            .*

Hex dump of section '\.text\.2':
 NOTE: This section has relocations against it, but these have NOT been applied to this dump\.
  0x00000000 00000000 00000000 08000000 0c000000 .*
  0x00000010 4054ffff c38d7600 00000000 00000000 .*
  0x00000020 20000000 24000000 4054ffff          .*
