#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX break
#as: -march=r2

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c009      	break.n	0
0+0002 <[^>]*> c009      	break.n	0
0+0004 <[^>]*> c7c9      	break.n	31
0+0006 <[^>]*> c389      	break.n	14
