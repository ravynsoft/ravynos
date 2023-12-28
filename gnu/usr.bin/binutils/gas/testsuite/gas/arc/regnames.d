#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <r0>:
   0:	200a 0f80 0000 0014 	mov	r0,0x14
   8:	2000 0f80 0000 0000 	add	r0,r0,0
			c: R_ARC_32_ME	gp
  10:	1a00 3080           	st	r2,\[gp\]
			10: R_ARC_SDA_LDST	.text\+0x14
