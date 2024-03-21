#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX mov.n
#as: -march=r2

# Test the mov.n and movi.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 213b      	mov.n	r4,r4
0+0002 <[^>]*> 011b      	movi.n	r4,0
0+0004 <[^>]*> 031b      	movi.n	r4,1
0+0006 <[^>]*> 7f1b      	movi.n	r4,63
0+0008 <[^>]*> f91b      	movi.n	r4,124
0+000a <[^>]*> fb1b      	movi.n	r4,255
0+000c <[^>]*> fd1b      	movi.n	r4,-2
0+000e <[^>]*> ff1b      	movi.n	r4,-1
