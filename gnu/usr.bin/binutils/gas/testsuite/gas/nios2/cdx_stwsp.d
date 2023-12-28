#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX stwsp.n
#as: -march=r2

# Test the stwsp.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2033      	stwsp.n	r4,0\(sp\)
0+0002 <[^>]*> 2073      	stwsp.n	r4,4\(sp\)
0+0004 <[^>]*> 23f3      	stwsp.n	r4,60\(sp\)
0+0006 <[^>]*> 27f3      	stwsp.n	r4,124\(sp\)
