#objdump: -sr
#name: Valid cross-section PC-relative references (o32)
#as: -32 -EB
#source: pcrel-4.s

.*:     file format .*

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
00000000 R_MIPS_PC32       foo
00000004 R_MIPS_PC32       foo
00000008 R_MIPS_PC32       foo
0000000c R_MIPS_PC32       foo

#...
Contents of section \.data:
 0000 00000000 00000004 00000008 fffffff0  ................
#pass
