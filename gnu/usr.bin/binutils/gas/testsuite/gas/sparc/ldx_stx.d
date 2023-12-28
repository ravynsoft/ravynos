#as: -Av9
#objdump: -dr -m sparc:v9
#name: sparc LDX/STX

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	d0 58 c0 00 	ldx  \[ %g3 \], %o0
   4:	d4 d8 c0 80 	ldxa  \[ %g3 \] #ASI_N, %o2
   8:	c3 08 c0 00 	ldx  \[ %g3 \], %fsr
   c:	d0 70 c0 00 	stx  %o0, \[ %g3 \]
  10:	d4 f0 c0 80 	stxa  %o2, \[ %g3 \] #ASI_N
  14:	c3 28 c0 00 	stx  %fsr, \[ %g3 \]
