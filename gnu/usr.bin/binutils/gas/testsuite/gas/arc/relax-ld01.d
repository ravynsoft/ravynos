#as: -mrelax
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
   0:	78e0                	nop_s
   2:	1710 7001           	ld	r1,\[pcl,16\]\s.*
   6:	264a 7000\s+.*
   a:	264a 7000\s+.*
   e:	78e0                	nop_s
  10:	2000 0000           	add	r0,r0,r0
