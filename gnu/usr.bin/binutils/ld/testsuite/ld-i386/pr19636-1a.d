#source: pr19636-1.s
#as: --32 -mrelax-relocations=no
#ld: -pie -m elf_i386 --no-dynamic-linker
#readelf : -r --wide -x .got -x .got.plt --dyn-syms

There are no relocations in this file.

Symbol table '\.dynsym' contains 1 entry:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +

Hex dump of section '.got':
  0x[0-9a-f]+ [0 ]+\.+

Hex dump of section '.got.plt':
  0x[0-9a-f]+ +[0-9a-f]+ +[0 ]+ .*
