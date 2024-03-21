#source: pr19636-2.s
#as: --64 -mrelax-relocations=no
#ld: -pie -m elf_x86_64 --no-dynamic-linker -z dynamic-undefined-weak
#readelf : -r --wide -x .got -x .got.plt
#warning: -z dynamic-undefined-weak ignored

There are no relocations in this file.

Hex dump of section '.got':
  0x[0-9a-f]+ [0 ]+\.+

Hex dump of section '.got.plt':
  0x[0-9a-f]+ +[0-9a-f]+ +[0 ]+ .+
  0x[0-9a-f]+ [0 ]+\.+
