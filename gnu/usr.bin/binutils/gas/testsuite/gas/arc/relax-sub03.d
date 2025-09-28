#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	264a 7000\s+.*
   4:	2242 0201           	sub	r1,r2,0x8
   8:	264a 7000\s+.*
   c:	2000 0000           	add	r0,r0,r0
