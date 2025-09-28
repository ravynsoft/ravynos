#name: C6X data relocations, local symbols
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld
#source: data-reloc-local-1.s
#source: data-reloc-local-2.s
#objdump: -r -s -j .data

.*: *file format elf32-tic6x-le

Contents of section \.data:
[ \t]*0080 80000000 84000000 8c000000 88000000  .*
[ \t]*0090 8c00888c                             .*
