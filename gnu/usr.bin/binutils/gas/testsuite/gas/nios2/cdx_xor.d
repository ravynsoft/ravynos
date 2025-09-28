#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX xor.n
#as: -march=r2

# Test the xor.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 3909      	xor.n	r4,r4,r4
0+0002 <[^>]*> 3e49      	xor.n	r17,r17,r7
0+0004 <[^>]*> 33c9      	xor.n	r7,r7,r17
0+0006 <[^>]*> 3fc9      	xor.n	r7,r7,r7
