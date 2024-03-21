#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 tret
#as: -march=r2
#source: tret.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0400f760 	eret
