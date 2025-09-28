#name: C6X ABS relocs to undefined weak symbols
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tsbr.ld
#source: weak.s
#objdump: -s -j .data

.*: *file format elf32-tic6x-le


Contents of section \.data:
[ \t]+0+80 00000000 00000000[ \t]+\.*[ \t]*
