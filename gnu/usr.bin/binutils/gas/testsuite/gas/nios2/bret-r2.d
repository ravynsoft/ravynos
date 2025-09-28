#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 bret
#as: -march=r2
#source: bret.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 240007a0 	bret

