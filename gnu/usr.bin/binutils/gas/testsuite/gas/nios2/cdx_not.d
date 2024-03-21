#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX not.n
#as: -march=r2

# Test the not.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 6909      	not.n	r4,r4
0+0002 <[^>]*> 63c9      	not.n	r17,r7
0+0004 <[^>]*> 6e49      	not.n	r7,r17
0+0006 <[^>]*> 6fc9      	not.n	r7,r7
