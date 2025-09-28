#name: C6X data relocations, global symbols
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s0=0 --defsym sff=0xff --defsym sffff=0xffff --defsym s80000000=0x80000000 --defsym sffff8000=0xffff8000 --defsym sffffff80=0xffffff80 --defsym sffffffff=0xffffffff
#source: data-reloc-global.s
#objdump: -r -s -j .data

.*: *file format elf32-tic6x-le

Contents of section \.data:
[ \t]*0080 00000080 ffffffff 00000000 feffffff  .*
[ \t]*0090 00000000 ffffffff 00800080 0000ffff  .*
[ \t]*00a0 808000                               .*
