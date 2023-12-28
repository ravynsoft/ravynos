#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX jmpr.n
#as: -march=r2

# Test the jmpr.n instruction.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> a1c9      	jmpr.n	r7
	...
