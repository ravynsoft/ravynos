#name: C6X MVK relocations, global symbols, REL
#as: -mlittle-endian -mgenerate-rel
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s0=0 --defsym s7fff=0x7fff --defsym s80000000=0x80000000 --defsym sffff8000=0xffff8000 --defsym sffffffff=0xffffffff
#source: mvk-reloc-global-rel.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

10000000 <[^>]*>:
10000000:[ \t]+00800028[ \t]+mvk \.S1 0,a1
10000004:[ \t]+00ffffa8[ \t]+mvk \.S1 -1,a1
10000008:[ \t]+00800028[ \t]+mvk \.S1 0,a1
1000000c:[ \t]+00ffff28[ \t]+mvk \.S1 -2,a1
10000010:[ \t]+00c00028[ \t]+mvk \.S1 -32768,a1
10000014:[ \t]+00c00028[ \t]+mvk \.S1 -32768,a1
10000018:[ \t]+00bfffa8[ \t]+mvk \.S1 32767,a1
1000001c:[ \t]+00000000[ \t]+nop 1
