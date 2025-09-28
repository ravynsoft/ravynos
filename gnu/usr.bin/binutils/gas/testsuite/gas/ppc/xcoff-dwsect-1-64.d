#as: -a64
#source: xcoff-dwsect-1.s
#objdump: -j .dwinfo -s
#name: XCOFF dwsect test 1 (64-bit)

.*:     file format aix.*coff64-rs6000

Contents of section \.dwinfo:
 0000 ffffffff 00000000 00000006 00020001  ................
 0010 0004ffff ffff0000 00000000 00020003  ................
