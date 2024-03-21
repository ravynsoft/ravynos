#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX ldwsp.n
#as: -march=r2

# Test the ld instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2013      	ldwsp.n	r4,0\(sp\)
0+0002 <[^>]*> 2053      	ldwsp.n	r4,4\(sp\)
0+0004 <[^>]*> 23d3      	ldwsp.n	r4,60\(sp\)
0+0006 <[^>]*> 27d3      	ldwsp.n	r4,124\(sp\)
