#name: C6X data relocations, local symbols, -r
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: data-reloc-local-1.s
#source: data-reloc-local-2.s
#objdump: -r -s -j .data

.*: *file format elf32-tic6x-le

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
0+ R_C6000_ABS32     \.data
0+4 R_C6000_ABS32     \.data\+0x00000004
0+8 R_C6000_ABS32     \.data\+0x0000000c
0+c R_C6000_ABS32     \.data\+0x00000008
0+10 R_C6000_ABS16     \.data\+0x0000000c
0+12 R_C6000_ABS8      \.data\+0x00000008
0+13 R_C6000_ABS8      \.data\+0x0000000c


Contents of section \.data:
[ \t]*0000 00000000 00000000 00000000 00000000  .*
[ \t]*0010 00000000                             .*
