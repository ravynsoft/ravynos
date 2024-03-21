#source: pr19636-1.s
#as: --32 -mrelax-relocations=no
#ld: -pie -m elf_i386 --no-dynamic-linker -z dynamic-undefined-weak
#readelf : -r --wide -x .got -x .got.plt
#warning: -z dynamic-undefined-weak ignored

There are no relocations in this file.

Hex dump of section '.got':
  0x[0-9a-f]+ [0 ]+\.+

Hex dump of section '.got.plt':
  0x[0-9a-f]+ +[0-9a-f]+ +[0 ]+ .*
