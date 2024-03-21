#name: C6X data relocations, local symbols, -r, REL
#as: -mlittle-endian -mgenerate-rel
#ld: -r -melf32_tic6x_le
#source: data-reloc-local-1.s
#source: data-reloc-local-2.s
#objdump: -r -s -j .data

.*: *file format elf32-tic6x-le

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
0+ R_C6000_ABS32     \.data
0+4 R_C6000_ABS32     \.data
0+8 R_C6000_ABS32     \.data
0+c R_C6000_ABS32     \.data
0+10 R_C6000_ABS16     \.data
0+12 R_C6000_ABS8      \.data
0+13 R_C6000_ABS8      \.data


Contents of section \.data:
[ \t]*0000 00000000 04000000 0c000000 08000000  \.\.\.\.\.\.\.\.\.\.\.\.\.\.\.\.
[ \t]*0010 0c00080c                             \.\.\.\.            
