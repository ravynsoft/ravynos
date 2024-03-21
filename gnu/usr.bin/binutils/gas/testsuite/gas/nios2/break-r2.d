#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 break
#as: -march=r2
#source: break.s

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> d01e0020 	break	0
0+0004 <[^>]*> d01e0020 	break	0
0+0008 <[^>]*> d3fe0020 	break	31
0+000c <[^>]*> d1de0020 	break	14


