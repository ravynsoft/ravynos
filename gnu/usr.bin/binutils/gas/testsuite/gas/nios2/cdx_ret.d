#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX ret.n
#as: -march=r2

# Test the ret.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> e009      	ret.n
	...