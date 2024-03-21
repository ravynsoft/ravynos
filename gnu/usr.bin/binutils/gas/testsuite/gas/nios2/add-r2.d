#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 add
#as: -march=r2
#source: add.s

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> c4042120 	add	r4,r4,r4
0+0004 <[^>]*> 7fff2104 	addi	r4,r4,32767
0+0008 <[^>]*> 80002104 	addi	r4,r4,-32768
0+000c <[^>]*> 00002104 	addi	r4,r4,0
0+0010 <[^>]*> ffff2104 	addi	r4,r4,-1
0+0014 <[^>]*> ffff2104 	addi	r4,r4,-1
0+0018 <[^>]*> 34562104 	addi	r4,r4,13398
0+001c <[^>]*> c4000020 	nop
