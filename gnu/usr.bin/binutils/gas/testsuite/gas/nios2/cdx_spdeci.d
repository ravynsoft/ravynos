#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX spdeci.n
#as: -march=r2

# Test the spdeci.n and spinci.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 8029      	spdeci.n	0
0+0002 <[^>]*> 9fe9      	spdeci.n	508
0+0004 <[^>]*> 0029      	spinci.n	0
0+0006 <[^>]*> 1fe9      	spinci.n	508
