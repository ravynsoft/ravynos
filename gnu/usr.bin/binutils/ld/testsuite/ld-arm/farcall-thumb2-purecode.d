.*:     file format .*

Disassembly of section .text:

00001000 <bar>:
    1000:	4770      	bx	lr

Disassembly of section .foo:

02001020 <_start>:
 2001020:	f000 f802 	bl	2001028 <__bar_veneer>
 2001024:	0000      	movs	r0, r0
	\.\.\.

02001028 <__bar_veneer>:
 2001028:	f241 0c01 	movw	ip, #4097	@ 0x1001
 200102c:	f2c0 0c00 	movt	ip, #0
 2001030:	4760      	bx	ip
 2001032:	0000      	movs	r0, r0
 2001034:	0000      	movs	r0, r0
	\.\.\.

