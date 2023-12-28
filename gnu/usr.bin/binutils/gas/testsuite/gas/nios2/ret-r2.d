#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 ret
#as: -march=r2
#source: ret.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 140007e0 	ret

