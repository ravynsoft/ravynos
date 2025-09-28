#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	78e0                	nop_s
   2:	8223                	ld_s	r1,\[r2,0xc\]
   4:	264a 7000\s+.*
   8:	264a 7000\s+.*
   c:	2000 0000           	add	r0,r0,r0
