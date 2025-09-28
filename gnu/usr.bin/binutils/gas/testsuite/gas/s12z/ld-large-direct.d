#objdump: -d
#name:    LD reg - conldant optimised to EXT24
#source:  ld-large-direct.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	b0 12 34 56 	ld d2, 1193046
   4:	b1 c0 ff ee 	ld d3, 12648430
   8:	b2 80 00 02 	ld d4, 8388610
   c:	b4 80 00 03 	ld d0, 8388611
  10:	b5 80 00 03 	ld d1, 8388611
  14:	b3 80 00 04 	ld d5, 8388612
  18:	b6 80 00 06 	ld d6, 8388614
  1c:	b7 80 00 07 	ld d7, 8388615
  20:	b8 80 00 08 	ld x, 8388616
  24:	b9 80 00 09 	ld y, 8388617
