#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX trap
#as: -march=r2

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> d009      	trap.n	0
0+0002 <[^>]*> d009      	trap.n	0
0+0004 <[^>]*> d7c9      	trap.n	31
0+0006 <[^>]*> d389      	trap.n	14
