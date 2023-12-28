#name: C6X GOT relocations, no overflow
#as: -mlittle-endian -mdsbt -mpic -mpid=near
#ld: -melf32_tic6x_le -Tdsbt-inrange.ld --dsbt-index 4 -shared
#source: got-reloc-global.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

10000000 <[^>]*>:
10000000:[ \t]+0700046e[ \t]+ldw \.D2T2 \*\+b14\(16\),b14
10000004:[ \t]+00ffff6c[ \t]+ldw \.D2T1 \*\+b14\(131068\),a1
[ \t]+...
