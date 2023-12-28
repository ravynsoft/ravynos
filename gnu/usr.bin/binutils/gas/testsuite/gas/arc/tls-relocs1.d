#as: -mcpu=archs
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <foo>:
   0:	2100 3f82 0000 0000 	add	r2,r25,0
			4: R_ARC_TLS_LE_32	a\+0x30
   8:	2000 0f80 0000 003c 	add	r0,r0,0x3c
