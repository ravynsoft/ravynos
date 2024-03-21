#objdump: -r
#name: Valid cross-section PC-relative references (n32)
#as: -n32 -mips3
#source: pcrel-4.s

.*:     file format .*

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
00000000 R_MIPS_PC32       foo
00000004 R_MIPS_PC32       foo\+0x00000004
00000008 R_MIPS_PC32       foo\+0x00000008
0000000c R_MIPS_PC32       foo-0x00000010
