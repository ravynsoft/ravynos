#source: pr19636-2.s
#as: --32 -mrelax-relocations=no
#ld: -pie -E -m elf_i386 -z notext
#readelf : -r --wide -x .got -x .got.plt --dyn-syms

Relocation section '\.rel\.dyn' at offset [0x0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym. Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_PC32 +0+ +func

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func
#...

Hex dump of section '.got':
  0x[0-9a-f]+ [0 ]+\.+

Hex dump of section '.got.plt':
  0x[0-9a-f]+ +[0-9a-f]+ +[0 ]+ .*
