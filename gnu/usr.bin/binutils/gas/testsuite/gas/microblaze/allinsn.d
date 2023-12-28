#as:
#objdump: -d

.*: +file format .*

Disassembly of section .text:

00000000 <lbur>:
   0:	c0000200 	lbur	r0, r0, r0

00000004 <lhur>:
   4:	c4000200 	lhur	r0, r0, r0

00000008 <lwr>:
   8:	c8000200 	lwr	r0, r0, r0

0000000c <sbr>:
   c:	d0000200 	sbr	r0, r0, r0

00000010 <shr>:
  10:	d4000200 	shr	r0, r0, r0

00000014 <swr>:
  14:	d8000200 	swr	r0, r0, r0

00000018 <clz>:
  18:	900000e0 	clz	r0, r0

0000001c <mbar>:
  1c:	b8420004 	mbar	2

00000020 <sleep>:
  20:	ba020004 	sleep

00000024 <regslr>:
  24:	b0000000 	imm	0
  28:	31600000 	addik	r11, r0, 0
  2c:	940bc800 	mts	rslr, r11

00000030 <regshr>:
  30:	b0000000 	imm	0
  34:	31600000 	addik	r11, r0, 0
  38:	940bc802 	mts	rshr, r11

0000003c <swapb>:
  3c:	900001e0 	swapb	r0, r0

00000040 <swaph>:
  40:	900001e2 	swaph	r0, r0
