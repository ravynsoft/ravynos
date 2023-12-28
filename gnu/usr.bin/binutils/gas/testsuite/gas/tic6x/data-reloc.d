#objdump: -r
#name: C6X data relocations

.*: *file format elf32-tic6x-le

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
0+00 R_C6000_ABS32 +ext1
0+04 R_C6000_ABS32 +ext1\+0x0+04
0+08 R_C6000_ABS16 +ext2
0+0a R_C6000_ABS16 +ext2-0x0+2
0+0c R_C6000_ABS8 +ext3
0+0d R_C6000_ABS8 +ext3\+0x0+01
