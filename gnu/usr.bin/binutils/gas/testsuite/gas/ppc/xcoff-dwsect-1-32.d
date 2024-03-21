#as: -a32
#source: xcoff-dwsect-1.s
#objdump: -j .dwinfo -s
#name: XCOFF dwsect test 1 (32-bit)

.*:     file format aixcoff-rs6000

Contents of section \.dwinfo:
 0000 00000006 00020001 00040000 00020003  ................
