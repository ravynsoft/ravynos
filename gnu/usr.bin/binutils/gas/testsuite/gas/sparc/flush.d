#as: -Av9
#objdump: -dr
#name: sparc FLUSH

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	81 d8 40 02 	flush  \[ %g1 \+ %g2 \]
   4:	81 d8 40 02 	flush  \[ %g1 \+ %g2 \]
   8:	81 d8 c0 00 	flush  \[ %g3 \]
   c:	81 d8 c0 00 	flush  \[ %g3 \]
  10:	81 d9 20 80 	flush  \[ %g4 \+ 0x80 \]
  14:	81 d9 20 80 	flush  \[ %g4 \+ 0x80 \]
  18:	81 d9 60 90 	flush  \[ %g5 \+ 0x90 \]
  1c:	81 d9 60 90 	flush  \[ %g5 \+ 0x90 \]
