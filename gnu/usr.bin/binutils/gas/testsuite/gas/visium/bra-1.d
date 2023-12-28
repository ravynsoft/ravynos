#as: -mtune=gr6
#objdump: -d
#name: bra-1

.*: +file format .*

Disassembly of section .text:

00000000 <foo>:
	...
   8:	ff 95 54 04 	bra     tr,r21,r21
   c:	00 00 00 00 	nop
