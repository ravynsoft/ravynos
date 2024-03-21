#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 comments
#as: -march=r2
#source: comments.s

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00180002 	br	0000001c <start>
0+0004 <[^>]*> 00000002 	br	00000008 <abort>
0+0008 <[^>]*> 00001814 	movui	r3,0
0+000c <[^>]*> 00011014 	movui	r2,1
0+0010 <[^>]*> 00001814 	movui	r3,0
0+0014 <[^>]*> 00001014 	movui	r2,0
0+0018 <[^>]*> 00280002 	br	00000044 <exit>
0+001c <[^>]*> fffc1084 	addi	r2,r2,-4
0+0020 <[^>]*> 00015814 	movui	r11,1
0+0024 <[^>]*> 00002814 	movui	r5,0
0+0028 <[^>]*> 00003014 	movui	r6,0
0+002c <[^>]*> 00000002 	br	00000030 <ldst>
0+0030 <[^>]*> f00c1014 	movui	r2,61452
0+0034 <[^>]*> facea014 	movui	r20,64206
0+0038 <[^>]*> 0000a0b7 	stw	r20,0\(r2\)
0+003c <[^>]*> 0000a897 	ldw	r21,0\(r2\)
0+0040 <[^>]*> ffcc0002 	br	00000010 <end>
0+0044 <[^>]*> fffc0002 	br	00000044 <exit>
