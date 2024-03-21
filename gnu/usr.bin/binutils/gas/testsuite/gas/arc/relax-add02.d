#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	2540 0401           	add	r1,r5,0x10
   4:	264a 7000\s+.*
   8:	264a 7000\s+.*
   c:	264a 7000\s+.*
  10:	2000 0000           	add	r0,r0,r0
