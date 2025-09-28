#as: -Av9
#objdump: -dr
#name: sparc LDTW/STTW

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	d0 18 c0 00 	ldtw  \[ %g3 \], %o0
   4:	d4 98 c0 80 	ldtwa  \[ %g3 \] #ASI_N, %o2
   8:	d8 98 e0 00 	ldtwa  \[ %g3 \] %asi, %o4
   c:	d0 38 c0 00 	sttw  %o0, \[ %g3 \]
  10:	d4 b8 c0 80 	sttwa  %o2, \[ %g3 \] #ASI_N
  14:	d8 b8 e0 00 	sttwa  %o4, \[ %g3 \] %asi
