#as: -mtune=gr6
#objdump: -d
#name: brr-2

.*: +file format .*

Disassembly of section .text:

00000000 <foo>:
   0:	00 00 00 00 	nop
   4:	78 00 ff ff 	brr     tr,-1
   8:	84 c1 00 01 	moviq   r1,1
   c:	00 00 00 00 	nop
  10:	78 00 ff ff 	brr     tr,-1
  14:	84 c1 00 02 	moviq   r1,2
  18:	78 00 ff fa 	brr     tr,-6
  1c:	84 c1 00 04 	moviq   r1,4
  20:	00 00 00 00 	nop
