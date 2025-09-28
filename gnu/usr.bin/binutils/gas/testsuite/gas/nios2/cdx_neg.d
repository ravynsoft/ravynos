#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX neg.n
#as: -march=r2

# Test the neg.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 7909      	neg.n	r4,r4
0+0002 <[^>]*> 7e49      	neg.n	r17,r7
0+0004 <[^>]*> 73c9      	neg.n	r7,r17
0+0006 <[^>]*> 7fc9      	neg.n	r7,r7
