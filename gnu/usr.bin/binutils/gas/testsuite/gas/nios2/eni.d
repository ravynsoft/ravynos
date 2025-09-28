#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 eni
#as: -march=r2

# Test the eni instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 20000020 	eni	0
0+0004 <[^>]*> 20000020 	eni	0
0+0008 <[^>]*> 20200020 	eni	1
