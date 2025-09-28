#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX sub.n
#as: -march=r2

# Test the sub.n and subi.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c901      	sub.n	r4,r4,r4
0+0002 <[^>]*> 8911      	subi.n	r4,r4,1
0+0004 <[^>]*> 9911      	subi.n	r4,r4,2
0+0006 <[^>]*> a911      	subi.n	r4,r4,4
0+0008 <[^>]*> b911      	subi.n	r4,r4,8
0+000a <[^>]*> c911      	subi.n	r4,r4,16
0+000c <[^>]*> d911      	subi.n	r4,r4,32
0+000e <[^>]*> e911      	subi.n	r4,r4,64
0+0010 <[^>]*> f911      	subi.n	r4,r4,128
	...
