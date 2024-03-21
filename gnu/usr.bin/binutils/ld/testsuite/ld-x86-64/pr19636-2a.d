#source: pr19636-2.s
#as: --64 -mrelax-relocations=no
#ld: -pie -m elf_x86_64 --no-dynamic-linker
#readelf : -r --wide -x .got -x .got.plt --dyn-syms

There are no relocations in this file.

Symbol table '\.dynsym' contains 1 entry:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +

Hex dump of section '.got':
  0x[0-9a-f]+ [0 ]+\.+

Hex dump of section '.got.plt':
  0x[0-9a-f]+ +[0-9a-f]+ +[0 ]+ .+
  0x[0-9a-f]+ [0 ]+\.+
