#as: -Av9m8
#objdump: -dr
#name: OSA2017 RLE instructions


.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	87 b0 46 02 	rle_burst  %g1, %g2, %g3
   4:	85 b0 06 41 	rle_length  %g1, %g2
