#source: pr19636-1.s
#as: --32 -mrelax-relocations=no
#ld: -pie -E -m elf_i386 --no-dynamic-linker
#readelf : -r --wide -x .got -x .got.plt --dyn-syms

There are no relocations in this file.

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +GLOBAL +DEFAULT +[0-9]+ +_start
#...

Hex dump of section '.got':
  0x[0-9a-f]+ [0 ]+\.+

Hex dump of section '.got.plt':
  0x[0-9a-f]+ +[0-9a-f]+ +[0 ]+ .*
