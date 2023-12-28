#as: -Av9 -relax
#source: call-relax.s
#objdump: -dr
#name: sparc relax CALL (a.out)

.*: +file format .*a\.out.*

Disassembly of section .text:

0+ <foo>:
   0:	31 00 00 00 	sethi  %hi\(0\), %i0
   4:	10 80 00 02 	b  c <bar>
   8:	91 ee 20 00 	restore  %i0, 0, %o0

0+c <bar>:
   c:	31 00 00 00 	sethi  %hi\(0\), %i0
  10:	40 00 00 00 	call  10 <bar\+0x4>
			10: WDISP30	_undefined-0x10
  14:	91 ee 20 00 	restore  %i0, 0, %o0
