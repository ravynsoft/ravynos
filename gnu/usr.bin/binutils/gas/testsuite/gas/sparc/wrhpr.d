#as: -64 -Av9m
#objdump: -dr
#name: sparc64 wrhpr

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	81 98 40 02 	wrhpr  %g1, %g2, %hpstate
   4:	81 98 40 00 	wrhpr  %g1, %hpstate
   8:	81 98 62 9a 	wrhpr  %g1, 0x29a, %hpstate
   c:	81 98 62 9a 	wrhpr  %g1, 0x29a, %hpstate
  10:	81 98 22 9a 	wrhpr  0x29a, %hpstate
  14:	83 98 40 02 	wrhpr  %g1, %g2, %htstate
  18:	83 98 40 00 	wrhpr  %g1, %htstate
  1c:	83 98 62 9a 	wrhpr  %g1, 0x29a, %htstate
  20:	83 98 62 9a 	wrhpr  %g1, 0x29a, %htstate
  24:	83 98 22 9a 	wrhpr  0x29a, %htstate
  28:	87 98 40 02 	wrhpr  %g1, %g2, %hintp
  2c:	87 98 40 00 	wrhpr  %g1, %hintp
  30:	87 98 62 9a 	wrhpr  %g1, 0x29a, %hintp
  34:	87 98 62 9a 	wrhpr  %g1, 0x29a, %hintp
  38:	87 98 22 9a 	wrhpr  0x29a, %hintp
  3c:	8b 98 40 02 	wrhpr  %g1, %g2, %htba
  40:	8b 98 40 00 	wrhpr  %g1, %htba
  44:	8b 98 62 9a 	wrhpr  %g1, 0x29a, %htba
  48:	8b 98 62 9a 	wrhpr  %g1, 0x29a, %htba
  4c:	8b 98 22 9a 	wrhpr  0x29a, %htba
  50:	af 98 40 02 	wrhpr  %g1, %g2, %hmcdper
  54:	af 98 40 00 	wrhpr  %g1, %hmcdper
  58:	af 98 62 9a 	wrhpr  %g1, 0x29a, %hmcdper
  5c:	af 98 62 9a 	wrhpr  %g1, 0x29a, %hmcdper
  60:	af 98 22 9a 	wrhpr  0x29a, %hmcdper
  64:	b1 98 40 02 	wrhpr  %g1, %g2, %hmcddfr
  68:	b1 98 40 00 	wrhpr  %g1, %hmcddfr
  6c:	b1 98 62 9a 	wrhpr  %g1, 0x29a, %hmcddfr
  70:	b1 98 62 9a 	wrhpr  %g1, 0x29a, %hmcddfr
  74:	b1 98 22 9a 	wrhpr  0x29a, %hmcddfr
  78:	b7 98 40 02 	wrhpr  %g1, %g2, %hva_mask_nz
  7c:	b7 98 40 00 	wrhpr  %g1, %hva_mask_nz
  80:	b7 98 62 9a 	wrhpr  %g1, 0x29a, %hva_mask_nz
  84:	b7 98 62 9a 	wrhpr  %g1, 0x29a, %hva_mask_nz
  88:	b7 98 22 9a 	wrhpr  0x29a, %hva_mask_nz
  8c:	b9 98 40 02 	wrhpr  %g1, %g2, %hstick_offset
  90:	b9 98 40 00 	wrhpr  %g1, %hstick_offset
  94:	b9 98 62 9a 	wrhpr  %g1, 0x29a, %hstick_offset
  98:	b9 98 62 9a 	wrhpr  %g1, 0x29a, %hstick_offset
  9c:	b9 98 22 9a 	wrhpr  0x29a, %hstick_offset
  a0:	bb 98 40 02 	wrhpr  %g1, %g2, %hstick_enable
  a4:	bb 98 40 00 	wrhpr  %g1, %hstick_enable
  a8:	bb 98 62 9a 	wrhpr  %g1, 0x29a, %hstick_enable
  ac:	bb 98 62 9a 	wrhpr  %g1, 0x29a, %hstick_enable
  b0:	bb 98 22 9a 	wrhpr  0x29a, %hstick_enable
  b4:	bf 98 40 02 	wrhpr  %g1, %g2, %hstick_cmpr
  b8:	bf 98 40 00 	wrhpr  %g1, %hstick_cmpr
  bc:	bf 98 62 9a 	wrhpr  %g1, 0x29a, %hstick_cmpr
  c0:	bf 98 62 9a 	wrhpr  %g1, 0x29a, %hstick_cmpr
  c4:	bf 98 22 9a 	wrhpr  0x29a, %hstick_cmpr
