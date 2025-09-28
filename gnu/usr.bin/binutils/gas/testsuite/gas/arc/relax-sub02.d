#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	2742 7401           	sub	r1,pcl,0x10\s.*
   4:	264a 7000\s+.*
   8:	264a 7000\s+.*
   c:	264a 7000\s+.*
  10:	2000 0000           	add	r0,r0,r0
