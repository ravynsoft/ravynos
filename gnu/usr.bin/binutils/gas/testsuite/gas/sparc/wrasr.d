#as: -64 -Av9m
#objdump: -dr
#name: sparc64 wrasr

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	a1 80 40 02 	wr  %g1, %g2, %pcr
   4:	a1 80 62 9a 	wr  %g1, 0x29a, %pcr
   8:	a1 80 62 9a 	wr  %g1, 0x29a, %pcr
   c:	a3 80 40 02 	wr  %g1, %g2, %pic
  10:	a3 80 62 9a 	wr  %g1, 0x29a, %pic
  14:	a3 80 62 9a 	wr  %g1, 0x29a, %pic
  18:	a5 80 40 02 	wr  %g1, %g2, %dcr
  1c:	a5 80 62 9a 	wr  %g1, 0x29a, %dcr
  20:	a5 80 62 9a 	wr  %g1, 0x29a, %dcr
  24:	a7 80 40 02 	wr  %g1, %g2, %gsr
  28:	a7 80 62 9a 	wr  %g1, 0x29a, %gsr
  2c:	a7 80 62 9a 	wr  %g1, 0x29a, %gsr
  30:	a9 80 40 02 	wr  %g1, %g2, %softint_set
  34:	a9 80 62 9a 	wr  %g1, 0x29a, %softint_set
  38:	a9 80 62 9a 	wr  %g1, 0x29a, %softint_set
  3c:	ab 80 40 02 	wr  %g1, %g2, %softint_clear
  40:	ab 80 62 9a 	wr  %g1, 0x29a, %softint_clear
  44:	ab 80 62 9a 	wr  %g1, 0x29a, %softint_clear
  48:	ad 80 40 02 	wr  %g1, %g2, %softint
  4c:	ad 80 62 9a 	wr  %g1, 0x29a, %softint
  50:	ad 80 62 9a 	wr  %g1, 0x29a, %softint
  54:	af 80 40 02 	wr  %g1, %g2, %tick_cmpr
  58:	af 80 62 9a 	wr  %g1, 0x29a, %tick_cmpr
  5c:	af 80 62 9a 	wr  %g1, 0x29a, %tick_cmpr
  60:	b1 80 40 02 	wr  %g1, %g2, %stick
  64:	b1 80 62 9a 	wr  %g1, 0x29a, %stick
  68:	b1 80 62 9a 	wr  %g1, 0x29a, %stick
  6c:	b3 80 40 02 	wr  %g1, %g2, %stick_cmpr
  70:	b3 80 62 9a 	wr  %g1, 0x29a, %stick_cmpr
  74:	b3 80 62 9a 	wr  %g1, 0x29a, %stick_cmpr
  78:	b5 80 40 02 	wr  %g1, %g2, %cfr
  7c:	b5 80 62 9a 	wr  %g1, 0x29a, %cfr
  80:	b5 80 62 9a 	wr  %g1, 0x29a, %cfr
  84:	b7 80 40 02 	wr  %g1, %g2, %pause
  88:	b7 80 62 9a 	wr  %g1, 0x29a, %pause
  8c:	b7 80 62 9a 	wr  %g1, 0x29a, %pause
  90:	b9 80 40 02 	wr  %g1, %g2, %mwait
  94:	b9 80 62 9a 	wr  %g1, 0x29a, %mwait
  98:	b9 80 62 9a 	wr  %g1, 0x29a, %mwait
