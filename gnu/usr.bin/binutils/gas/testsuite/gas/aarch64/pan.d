#objdump: -dr
#as: -march=armv8-a+pan

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d500419f 	msr	pan, #0x1
   4:	d500409f 	msr	pan, #0x0
   8:	d5184260 	msr	pan, x0
   c:	d5384261 	mrs	x1, pan
