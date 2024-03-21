#objdump: -r
#name: Valid cross-section PC-relative references (n64)
#as: -64 -mips3
#source: pcrel-4.s

.*:     file format .*

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
0+000 R_MIPS_PC32       foo
0+000 R_MIPS_NONE       \*ABS\*
0+000 R_MIPS_NONE       \*ABS\*
0+004 R_MIPS_PC32       foo\+0x0+004
0+004 R_MIPS_NONE       \*ABS\*\+0x0+004
0+004 R_MIPS_NONE       \*ABS\*\+0x0+004
0+008 R_MIPS_PC32       foo\+0x0+008
0+008 R_MIPS_NONE       \*ABS\*\+0x0+008
0+008 R_MIPS_NONE       \*ABS\*\+0x0+008
0+00c R_MIPS_PC32       foo-0x0+010
0+00c R_MIPS_NONE       \*ABS\*-0x0+010
0+00c R_MIPS_NONE       \*ABS\*-0x0+010
