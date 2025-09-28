#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 CDX sll.n
#as: -march=r2

# Test the sll.n and slli.n instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 4909      	sll.n	r4,r4,r4
0+0002 <[^>]*> 0919      	slli.n	r4,r4,1
0+0004 <[^>]*> 1919      	slli.n	r4,r4,2
0+0006 <[^>]*> 2919      	slli.n	r4,r4,3
0+0008 <[^>]*> 3919      	slli.n	r4,r4,8
0+000a <[^>]*> 4919      	slli.n	r4,r4,12
0+000c <[^>]*> 5919      	slli.n	r4,r4,16
0+000e <[^>]*> 6919      	slli.n	r4,r4,24
0+0010 <[^>]*> 7919      	slli.n	r4,r4,31
0+0012 <[^>]*> 41c9      	sll.n	r7,r7,r16
0+0014 <[^>]*> 4e09      	sll.n	r16,r16,r7
	...
