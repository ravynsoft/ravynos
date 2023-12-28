#source: dso-1.s
#source: dsov32-1.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -s -T

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+130 g    DF \.text	0+8 dsofn3
#...
0+12c g    DF \.text	0+2 dsofn
#...
Contents of section \.rela\.plt:
.* bc210000 0b020000 00000000           .*
Contents of section \.plt:
.* 84e20401 7e7a3f7a 04f26ffa bf09b005  .*
.* 00000000 00000000 00006f0d 0c000000  .*
.* 6ffabf09 b0053f7e 00000000 bf0ed4ff  .*
.* ffffb005                             .*
Contents of section \.text:
.* b0050000 bfbee2ff ffffb005           .*
Contents of section \.dynamic:
#...
Contents of section \.got:
.* 38210000 00000000 00000000 1e010000  .*
