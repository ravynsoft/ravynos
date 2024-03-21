#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 br.n
#as: -march=r2

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c4000020 	nop
0+0004 <[^>]*> ff43      	br.n	00000000 <foo>
	...
