#as: -32 -Av8
#objdump: -dr
#name: V8 mov/wr aliases

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	83 80 00 10 	wr  %l0, %asr1
   4:	81 80 00 10 	wr  %l0, %y
   8:	81 88 00 10 	wr  %l0, %psr
   c:	81 90 00 10 	wr  %l0, %wim
  10:	81 98 00 10 	wr  %l0, %tbr
  14:	83 80 00 00 	wr  %g0, %asr1
  18:	81 80 00 00 	wr  %g0, %y
  1c:	81 88 00 00 	wr  %g0, %psr
  20:	81 90 00 00 	wr  %g0, %wim
  24:	81 98 00 00 	wr  %g0, %tbr
  28:	83 80 20 00 	wr  %g0, %asr1
  2c:	81 80 20 00 	wr  %g0, %y
  30:	81 88 20 00 	wr  %g0, %psr
  34:	81 90 20 00 	wr  %g0, %wim
  38:	81 98 20 00 	wr  %g0, %tbr
  3c:	83 80 3f ff 	wr  -1, %asr1
  40:	81 80 3f ff 	wr  -1, %y
  44:	81 88 3f ff 	wr  -1, %psr
  48:	81 90 3f ff 	wr  -1, %wim
  4c:	81 98 3f ff 	wr  -1, %tbr
  50:	83 80 00 10 	wr  %l0, %asr1
  54:	81 80 00 10 	wr  %l0, %y
  58:	81 88 00 10 	wr  %l0, %psr
  5c:	81 90 00 10 	wr  %l0, %wim
  60:	81 98 00 10 	wr  %l0, %tbr
  64:	83 80 00 00 	wr  %g0, %asr1
  68:	81 80 00 00 	wr  %g0, %y
  6c:	81 88 00 00 	wr  %g0, %psr
  70:	81 90 00 00 	wr  %g0, %wim
  74:	81 98 00 00 	wr  %g0, %tbr
  78:	83 80 20 00 	wr  %g0, %asr1
  7c:	81 80 20 00 	wr  %g0, %y
  80:	81 88 20 00 	wr  %g0, %psr
  84:	81 90 20 00 	wr  %g0, %wim
  88:	81 98 20 00 	wr  %g0, %tbr
  8c:	83 80 3f ff 	wr  -1, %asr1
  90:	81 80 3f ff 	wr  -1, %y
  94:	81 88 3f ff 	wr  -1, %psr
  98:	81 90 3f ff 	wr  -1, %wim
  9c:	81 98 3f ff 	wr  -1, %tbr
