#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 comments

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 00000606 	br	0000001c <start>
0+0004 <[^>]*> 00000006 	br	00000008 <abort>
0+0008 <[^>]*> 00c00014 	movui	r3,0
0+000c <[^>]*> 00800054 	movui	r2,1
0+0010 <[^>]*> 00c00014 	movui	r3,0
0+0014 <[^>]*> 00800014 	movui	r2,0
0+0018 <[^>]*> 00000a06 	br	00000044 <exit>
0+001c <[^>]*> 10bfff04 	addi	r2,r2,-4
0+0020 <[^>]*> 02c00054 	movui	r11,1
0+0024 <[^>]*> 01400014 	movui	r5,0
0+0028 <[^>]*> 01800014 	movui	r6,0
0+002c <[^>]*> 00000006 	br	00000030 <ldst>
0+0030 <[^>]*> 00bc0314 	movui	r2,61452
0+0034 <[^>]*> 053eb394 	movui	r20,64206
0+0038 <[^>]*> 15000015 	stw	r20,0\(r2\)
0+003c <[^>]*> 15400017 	ldw	r21,0\(r2\)
0+0040 <[^>]*> 003ff306 	br	00000010 <end>
0+0044 <[^>]*> 003fff06 	br	00000044 <exit>
