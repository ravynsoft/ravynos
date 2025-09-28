#as: -64 -Av9m
#objdump: -dr
#name: sparc64 rdhpr

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	83 48 00 00 	rdhpr  %hpstate, %g1
   4:	85 48 40 00 	rdhpr  %htstate, %g2
   8:	87 48 c0 00 	rdhpr  %hintp, %g3
   c:	89 49 40 00 	rdhpr  %htba, %g4
  10:	8b 49 80 00 	rdhpr  %hver, %g5
  14:	8d 4f 00 00 	rdhpr  %hstick_offset, %g6
  18:	8b 4f 40 00 	rdhpr  %hstick_enable, %g5
  1c:	89 4f c0 00 	rdhpr  %hstick_cmpr, %g4
  20:	8d 4d c0 00 	rdhpr  %hmcdper, %g6
  24:	8b 4e 00 00 	rdhpr  %hmcddfr, %g5
  28:	89 4e c0 00 	rdhpr  %hva_mask_nz, %g4
