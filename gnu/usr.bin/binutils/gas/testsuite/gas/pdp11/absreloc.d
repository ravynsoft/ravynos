#name: pdp11 absreloc
#objdump: -drw

.*:     file format .*


Disassembly of section .text:

00000000 <start>:
   0:	0bf7 fffc      	tst	\$0 <start>
   4:	0bdf 0000      	tst	\*\$0	6: 16	\*ABS\*
   8:	0bf7 0008      	tst	\$14 <start\+0x14>	a: DISP16	\*ABS\*
   c:	0bdf 0014      	tst	\*\$24
  10:	0bf7 0000      	tst	\$14 <start\+0x14>	12: DISP16	\*ABS\*
  14:	0bdf 0014      	tst	\*\$24
