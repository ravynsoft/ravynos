#as: -Av9v
#objdump: -dr
#name: sparc CFR

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	b5 82 40 16 	wr  %o1, %l6, %cfr
   4:	b5 80 62 34 	wr  %g1, 0x234, %cfr
   8:	8b 46 80 00 	rd  %cfr, %g5
   c:	97 46 80 00 	rd  %cfr, %o3
  10:	b5 46 80 00 	rd  %cfr, %i2
  14:	a9 46 80 00 	rd  %cfr, %l4
