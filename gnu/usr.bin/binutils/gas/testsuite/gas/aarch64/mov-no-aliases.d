#source: mov.s
#objdump: -dr -Mno-aliases

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	110003ef 	add	w15, wsp, #0x0
   4:	910003ef 	add	x15, sp, #0x0
   8:	110000ff 	add	wsp, w7, #0x0
   c:	910000ff 	add	sp, x7, #0x0
  10:	110003ff 	add	wsp, wsp, #0x0
  14:	910003ff 	add	sp, sp, #0x0
  18:	aa0f03e7 	orr	x7, xzr, x15
  1c:	2a0f03e7 	orr	w7, wzr, w15
  20:	52800b01 	movz	w1, #0x58
  24:	12800000 	movn	w0, #0x0
  28:	b2607fe0 	orr	x0, xzr, #0xffffffff00000000
  2c:	b2400fff 	orr	sp, xzr, #0xf
  30:	32000fff 	orr	wsp, wzr, #0xf
  34:	d28001ff 	movz	xzr, #0xf
  38:	528001ff 	movz	wzr, #0xf
  3c:	0e1c3de7 	umov	w7, v15\.s\[3\]
  40:	4e183fef 	umov	x15, v31\.d\[1\]
  44:	d2801fe0 	movz	x0, #0xff
  48:	320de400 	orr	w0, w0, #0x99999999
