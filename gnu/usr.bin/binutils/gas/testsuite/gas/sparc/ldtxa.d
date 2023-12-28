#as: -Av9c
#objdump: -dr
#name: sparc LDTXA

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	d4 98 c4 40 	ldtxa  \[ %g3 \] #ASI_TWINX_AIUP, %o2
   4:	d4 98 c4 42 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_AIUP, %o2
   8:	d4 98 c4 60 	ldtxa  \[ %g3 \] #ASI_BLK_INIT_QUAD_LDD_AIUS, %o2
   c:	d4 98 c4 62 	ldtxa  \[ %g3 \+ %g2 \] #ASI_BLK_INIT_QUAD_LDD_AIUS, %o2
  10:	d4 98 c4 c0 	ldtxa  \[ %g3 \] #ASI_QUAD_LDD_PHYS_4V, %o2
  14:	d4 98 c4 c2 	ldtxa  \[ %g3 \+ %g2 \] #ASI_QUAD_LDD_PHYS_4V, %o2
  18:	d4 98 c4 e0 	ldtxa  \[ %g3 \] #ASI_TWINX_N, %o2
  1c:	d4 98 c4 e2 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_N, %o2
  20:	d4 98 c5 40 	ldtxa  \[ %g3 \] #ASI_TWINX_AIUP_L, %o2
  24:	d4 98 c5 42 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_AIUP_L, %o2
  28:	d4 98 c5 60 	ldtxa  \[ %g3 \] #ASI_TWINX_AIUS_L, %o2
  2c:	d4 98 c5 62 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_AIUS_L, %o2
  30:	d4 98 c5 c0 	ldtxa  \[ %g3 \] #ASI_TWINX_REAL_L, %o2
  34:	d4 98 c5 c2 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_REAL_L, %o2
  38:	d4 98 c5 e0 	ldtxa  \[ %g3 \] #ASI_TWINX_NL, %o2
  3c:	d4 98 c5 e2 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_NL, %o2
  40:	d4 98 dc 40 	ldtxa  \[ %g3 \] #ASI_BLK_INIT_QUAD_LDD_P, %o2
  44:	d4 98 dc 42 	ldtxa  \[ %g3 \+ %g2 \] #ASI_BLK_INIT_QUAD_LDD_P, %o2
  48:	d4 98 dc 60 	ldtxa  \[ %g3 \] #ASI_TWINX_S, %o2
  4c:	d4 98 dc 62 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_S, %o2
  50:	d4 98 dd 40 	ldtxa  \[ %g3 \] #ASI_TWINX_PL, %o2
  54:	d4 98 dd 42 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_PL, %o2
  58:	d4 98 dd 60 	ldtxa  \[ %g3 \] #ASI_TWINX_SL, %o2
  5c:	d4 98 dd 62 	ldtxa  \[ %g3 \+ %g2 \] #ASI_TWINX_SL, %o2