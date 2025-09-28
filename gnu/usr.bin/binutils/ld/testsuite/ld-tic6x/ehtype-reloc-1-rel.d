#name: EHTYPE relocations (REL)
#as: -mlittle-endian -mgenerate-rel
#ld: -melf32_tic6x_le --defsym s1=0x2a -Tdsbt-inrange.ld
#source: ehtype-reloc-1.s
#objdump: -s -j.data -j.text -j.got

.*: *file format elf32-tic6x-le

Contents of section .data:
 8018 78563412                             .*
Contents of section .text:
 10000000 08000000 0c000000                    .*
Contents of section .got:
 2001fff4 00000000 00000000 2a000000 18800000  .*
