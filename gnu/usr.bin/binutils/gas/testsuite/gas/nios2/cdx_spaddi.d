#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX spaddi.n
#as: -march=r2

# Test the spaddi.n instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 007d      	spaddi.n	r17,0
0+0002 <[^>]*> 01fd      	spaddi.n	r7,0
0+0004 <[^>]*> 7e7d      	spaddi.n	r17,252
0+0006 <[^>]*> 7ffd      	spaddi.n	r7,252
