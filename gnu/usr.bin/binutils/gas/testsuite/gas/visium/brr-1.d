#as:
#objdump: -d
#name: brr-1

.*: +file format .*

Disassembly of section .text:

00000000 <foo>:
   0:	78 00 00 00 	brr     tr,\+0
   4:	84 c1 00 01 	moviq   r1,1
   8:	78 00 00 00 	brr     tr,\+0
   c:	84 c1 00 02 	moviq   r1,2
  10:	78 00 ff fc 	brr     tr,-4
  14:	84 c1 00 04 	moviq   r1,4
  18:	00 00 00 00 	nop
