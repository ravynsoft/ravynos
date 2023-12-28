#as: -mcpu=archs -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <foo-0x4>:
   0:	78e0                	nop_s
   2:	78e0                	nop_s

00000004 <foo>:
   4:	2000 0000           	add	r0,r0,r0

00000008 <bar>:
   8:	ffff                	bl_s	-4	;4 <foo>
   a:	2100 0041           	add	r1,r1,r1
   e:	f1fc                	b_s	-8	;4 <foo>
