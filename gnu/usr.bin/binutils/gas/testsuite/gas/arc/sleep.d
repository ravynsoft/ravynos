#as: -mcpu=arc700
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

00000000 <main>:
   0:	2100 0080           	add	r0,r1,r2
   4:	216f 013f           	sleep	0x4
   8:	2402 0143           	sub	r3,r4,r5
