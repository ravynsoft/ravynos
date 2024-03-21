#objdump: -d
#name:    ST reg - constant optimised to EXT24
#source:  st-large-direct.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	d0 12 34 56 	st d2, 1193046
   4:	d1 c0 ff ee 	st d3, 12648430
   8:	d2 80 00 02 	st d4, 8388610
   c:	d4 80 00 03 	st d0, 8388611
  10:	d5 80 00 03 	st d1, 8388611
  14:	d3 80 00 04 	st d5, 8388612
  18:	d6 80 00 06 	st d6, 8388614
  1c:	d7 80 00 07 	st d7, 8388615
  20:	d8 80 00 08 	st x, 8388616
  24:	d9 80 00 09 	st y, 8388617
