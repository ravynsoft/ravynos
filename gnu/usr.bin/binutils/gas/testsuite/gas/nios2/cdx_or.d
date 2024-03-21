#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX or.n
#as: -march=r2

# Test the or.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2909      	or.n	r4,r4,r4
0+0002 <[^>]*> 2e49      	or.n	r17,r17,r7
0+0004 <[^>]*> 23c9      	or.n	r7,r7,r17
0+0006 <[^>]*> 2fc9      	or.n	r7,r7,r7
