#as: -mcpu=archs -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	78e0                	nop_s
   2:	240a 0f80 0000 0000 	mov	r4,0
			6: R_ARC_32_ME	.rodata
   a:	78e0                	nop_s
