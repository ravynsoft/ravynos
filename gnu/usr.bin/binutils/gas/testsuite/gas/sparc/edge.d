#as: -Av9d
#objdump: -dr
#name: sparc EDGE

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	87 b0 40 02 	edge8cc  %g1, %g2, %g3
   4:	87 b0 40 02 	edge8cc  %g1, %g2, %g3
   8:	87 b0 40 22 	edge8n  %g1, %g2, %g3
   c:	89 b0 80 43 	edge8lcc  %g2, %g3, %g4
  10:	89 b0 80 43 	edge8lcc  %g2, %g3, %g4
  14:	89 b0 80 63 	edge8ln  %g2, %g3, %g4
  18:	83 b5 00 02 	edge8cc  %l4, %g2, %g1
  1c:	83 b5 00 02 	edge8cc  %l4, %g2, %g1
  20:	83 b5 00 22 	edge8n  %l4, %g2, %g1
  24:	a9 b0 80 41 	edge8lcc  %g2, %g1, %l4
  28:	a9 b0 80 41 	edge8lcc  %g2, %g1, %l4
  2c:	a9 b0 80 61 	edge8ln  %g2, %g1, %l4
  30:	95 b3 41 0c 	edge32cc  %o5, %o4, %o2
  34:	95 b3 41 0c 	edge32cc  %o5, %o4, %o2
  38:	95 b3 41 2c 	edge32n  %o5, %o4, %o2
  3c:	a3 b2 81 45 	edge32lcc  %o2, %g5, %l1
  40:	a3 b2 81 45 	edge32lcc  %o2, %g5, %l1
  44:	a3 b2 81 65 	edge32ln  %o2, %g5, %l1
