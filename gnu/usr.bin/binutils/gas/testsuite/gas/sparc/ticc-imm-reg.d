#as: -32 -Av8
#objdump: -dr
#name: software traps

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	91 d2 00 00 	ta  %o0
   4:	91 d2 00 0a 	ta  %o0 \+ %o2
   8:	91 d4 20 0a 	ta  %l0 \+ 0xa
   c:	91 d4 3f f6 	ta  %l0 \+ -10
  10:	91 d4 3f f6 	ta  %l0 \+ -10
  14:	91 d4 20 0a 	ta  %l0 \+ 0xa
  18:	91 d0 20 7f 	ta  0x7f
  1c:	91 d6 20 0a 	ta  %i0 \+ 0xa
  20:	91 d6 3f f6 	ta  %i0 \+ -10
