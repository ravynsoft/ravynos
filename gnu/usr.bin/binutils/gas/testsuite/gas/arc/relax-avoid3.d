#as: -mcpu=archs -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <test>:
   0:	2000 0000           	add	r0,r0,r0

00000004 <main>:
   4:	0001 0000           	b	0	;0 <test>
			4: R_ARC_S25H_PCREL	test
