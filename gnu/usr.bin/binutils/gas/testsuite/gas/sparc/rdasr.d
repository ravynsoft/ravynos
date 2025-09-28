#as: -64 -Av9m8
#objdump: -dr
#name: sparc64 rdasr

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	81 44 00 00 	rd  %pcr, %g0
   4:	83 44 40 00 	rd  %pic, %g1
   8:	85 44 80 00 	rd  %dcr, %g2
   c:	87 44 c0 00 	rd  %gsr, %g3
  10:	89 45 80 00 	rd  %softint, %g4
  14:	8b 45 c0 00 	rd  %tick_cmpr, %g5
  18:	8b 46 00 00 	rd  %stick, %g5
  1c:	89 46 40 00 	rd  %stick_cmpr, %g4
  20:	8d 46 80 00 	rd  %cfr, %g6
  24:	85 43 40 00 	rd  %entropy, %g2
