#as: -Av8
#objdump: -dr -m sparc
#name: sparc LDFSR/STFSR

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	c1 08 c0 00 	ld  \[ %g3 \], %fsr
   4:	c1 28 c0 00 	st  %fsr, \[ %g3 \]
