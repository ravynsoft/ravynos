#as: -Av8
#objdump: -dr -m sparc
#name: sparc LDD/STD

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	d0 18 c0 00 	ldd  \[ %g3 \], %o0
   4:	d4 98 c0 80 	ldda  \[ %g3 \] #ASI_N, %o2
   8:	d0 38 c0 00 	std  %o0, \[ %g3 \]
   c:	d4 b8 c0 80 	stda  %o2, \[ %g3 \] #ASI_N
