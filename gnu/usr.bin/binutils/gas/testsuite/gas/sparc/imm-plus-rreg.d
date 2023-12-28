#as: -Av8
#objdump: -dr
#name: address: simm13 + rreg

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	c2 02 20 0a 	ld  \[ %o0 \+ 0xa \], %g1
   4:	c4 04 a0 0a 	ld  \[ %l2 \+ 0xa \], %g2
   8:	c4 22 20 0a 	st  %g2, \[ %o0 \+ 0xa \]
   c:	c2 24 a0 0a 	st  %g1, \[ %l2 \+ 0xa \]
