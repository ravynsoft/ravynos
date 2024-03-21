#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX and.n
#as: -march=r2

# Test the and.n and andi.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 0909      	and.n	r4,r4,r4
0+0002 <[^>]*> 090b      	andi.n	r4,r4,1
0+0004 <[^>]*> 190b      	andi.n	r4,r4,2
0+0006 <[^>]*> 290b      	andi.n	r4,r4,3
0+0008 <[^>]*> 390b      	andi.n	r4,r4,4
0+000a <[^>]*> 490b      	andi.n	r4,r4,8
0+000c <[^>]*> 590b      	andi.n	r4,r4,15
0+000e <[^>]*> 690b      	andi.n	r4,r4,16
0+0010 <[^>]*> 790b      	andi.n	r4,r4,31
0+0012 <[^>]*> 890b      	andi.n	r4,r4,32
0+0014 <[^>]*> 990b      	andi.n	r4,r4,63
0+0016 <[^>]*> a90b      	andi.n	r4,r4,127
0+0018 <[^>]*> b90b      	andi.n	r4,r4,128
0+001a <[^>]*> c90b      	andi.n	r4,r4,255
0+001c <[^>]*> d90b      	andi.n	r4,r4,2047
0+001e <[^>]*> e90b      	andi.n	r4,r4,65280
0+0020 <[^>]*> f90b      	andi.n	r4,r4,65535
	...
