#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	110003ef 	mov	w15, wsp
   4:	910003ef 	mov	x15, sp
   8:	110000ff 	mov	wsp, w7
   c:	910000ff 	mov	sp, x7
  10:	110003ff 	mov	wsp, wsp
  14:	910003ff 	mov	sp, sp
  18:	aa0f03e7 	mov	x7, x15
  1c:	2a0f03e7 	mov	w7, w15
  20:	52800b01 	mov	w1, #0x58                  	// #88
  24:	12800000 	mov	w0, #0xffffffff            	// #-1
  28:	b2607fe0 	mov	x0, #0xffffffff00000000    	// #-4294967296
  2c:	b2400fff 	mov	sp, #0xf                   	// #15
  30:	32000fff 	mov	wsp, #0xf                   	// #15
  34:	d28001ff 	mov	xzr, #0xf                   	// #15
  38:	528001ff 	mov	wzr, #0xf                   	// #15
  3c:	0e1c3de7 	mov	w7, v15\.s\[3\]
  40:	4e183fef 	mov	x15, v31\.d\[1\]
  44:	d2801fe0 	mov	x0, #0xff                  	// #255
  48:	320de400 	orr	w0, w0, #0x99999999
