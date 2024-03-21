#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	78e0                	nop_s
   2:	2742 7281           	sub	r1,pcl,0xa\s.*
   6:	264a 7000\s+.*
   a:	78e0                	nop_s
   c:	2000 0000           	add	r0,r0,r0
