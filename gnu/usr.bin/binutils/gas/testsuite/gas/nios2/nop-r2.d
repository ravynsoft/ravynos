#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 nop.n
#as: -march=r2

# Test the nop.n pseudo-instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 003b      	nop.n
	...
