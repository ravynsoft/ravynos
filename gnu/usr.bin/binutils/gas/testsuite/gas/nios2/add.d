#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 add

# Test the add instruction

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2109883a 	add	r4,r4,r4
0+0004 <[^>]*> 211fffc4 	addi	r4,r4,32767
0+0008 <[^>]*> 21200004 	addi	r4,r4,-32768
0+000c <[^>]*> 21000004 	addi	r4,r4,0
0+0010 <[^>]*> 213fffc4 	addi	r4,r4,-1
0+0014 <[^>]*> 213fffc4 	addi	r4,r4,-1
0+0018 <[^>]*> 210d1584 	addi	r4,r4,13398
0+001c <[^>]*> 0001883a 	nop
