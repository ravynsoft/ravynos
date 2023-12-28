#objdump: -dr
#as:
#name: PowerPC Test 2, 32 bit XCOFF

.*: +file format aixcoff-rs6000


Disassembly of section .text:

00000000 <.main>:
   0:	7c 08 02 a6 	mflr    r0
   4:	90 01 00 08 	st      r0,8\(r1\)
   8:	93 c1 ff f8 	st      r30,-8\(r1\)
   c:	93 e1 ff fc 	st      r31,-4\(r1\)
  10:	94 21 ff c0 	stu     r1,-64\(r1\)
  14:	3b e0 00 00 	lil     r31,0
  18:	83 c2 00 00 	l       r30,0\(r2\)
			1a: R_TOC	LC\.\.0-0x70
  1c:	7f c3 f3 78 	mr      r3,r30
  20:	7f e4 fb 78 	mr      r4,r31
  24:	4b ff ff dd 	bl      0 <.main>
			24: R_BR	.printf
  28:	60 00 00 00 	oril    r0,r0,0
  2c:	2f 9f 00 09 	cmpi    7,r31,9
  30:	3b ff 00 01 	cal     r31,1\(r31\)
  34:	40 9e ff e8 	bne     7,1c <\.main\+0x1c>
  38:	38 60 00 00 	lil     r3,0
  3c:	38 21 00 40 	cal     r1,64\(r1\)
  40:	80 01 00 08 	l       r0,8\(r1\)
  44:	7c 08 03 a6 	mtlr    r0
  48:	83 c1 ff f8 	l       r30,-8\(r1\)
  4c:	83 e1 ff fc 	l       r31,-4\(r1\)
  50:	4e 80 00 20 	br
  54:	60 00 00 00 	oril    r0,r0,0
  58:	60 00 00 00 	oril    r0,r0,0
  5c:	60 00 00 00 	oril    r0,r0,0

00000060 <_t\.rw_>:
  60:	25 64 0a 00 	dozi    r11,r4,2560
#pass
