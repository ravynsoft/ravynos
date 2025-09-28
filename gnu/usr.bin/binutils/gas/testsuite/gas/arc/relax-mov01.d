#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	78e0                	nop_s
   2:	258a 0300           	mov	r5,12
   6:	264a 7000\s+.*
   a:	264a 7000\s+.*
   e:	2000 0000           	add	r0,r0,r0
