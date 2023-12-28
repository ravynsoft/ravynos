#as: -Av9v
#objdump: -dr
#name: sparc PAUSE

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	b7 80 40 02 	wr  %g1, %g2, %pause
   4:	b7 83 22 34 	wr  %o4, 0x234, %pause
   8:	b7 80 20 08 	pause  8
   c:	b7 80 21 2c 	pause  0x12c
  10:	b7 80 22 34 	pause  0x234
