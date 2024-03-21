#objdump: -dr
#as: -march=armv8-a -mabi=lp64
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section .text:

0000000000000000 <.*>:
   0:	9ac32041 	lsl	x1, x2, x3
   4:	d340fc41 	lsr	x1, x2, #0
   8:	d37ff841 	lsl	x1, x2, #1
   c:	93c30441 	extr	x1, x2, x3, #1
  10:	93c3fc41 	extr	x1, x2, x3, #63
  14:	93c30041 	extr	x1, x2, x3, #0
  18:	13837c41 	extr	w1, w2, w3, #31
  1c:	9a9f17e1 	cset	x1, eq	// eq = none
  20:	da9f13e1 	csetm	x1, eq	// eq = none
  24:	71000021 	subs	w1, w1, #0x0
  28:	7100003f 	cmp	w1, #0x0
  2c:	4b0203e1 	neg	w1, w2
  30:	51000041 	sub	w1, w2, #0x0
  34:	f100003f 	cmp	x1, #0x0
  38:	f1000021 	subs	x1, x1, #0x0
  3c:	32000fe1 	orr	w1, wzr, #0xf
  40:	aa0203e1 	mov	x1, x2
  44:	18000061 	ldr	w1, 50 <sp>
  48:	18000621 	ldr	w1, 10c <sp\+0xbc>
  4c:	58000621 	ldr	x1, 110 <sp\+0xc0>

0000000000000050 <sp>:
  50:	12345678 	.word	0x12345678
  54:	d65f03c0 	ret
  58:	d65f03c0 	ret
  5c:	d65f0040 	ret	x2
  60:	8b22603f 	add	sp, x1, x2
  64:	91401ca5 	add	x5, x5, #0x7, lsl #12
  68:	8b430441 	add	x1, x2, x3, lsr #1
  6c:	91001ca5 	add	x5, x5, #0x7
  70:	71000421 	subs	w1, w1, #0x1
  74:	d2800c82 	mov	x2, #0x64                  	// #100
  78:	d2800c82 	mov	x2, #0x64                  	// #100
  7c:	d2800c82 	mov	x2, #0x64                  	// #100
  80:	d2a00c82 	mov	x2, #0x640000              	// #6553600
  84:	d2a00c82 	mov	x2, #0x640000              	// #6553600
  88:	d2c00c82 	mov	x2, #0x6400000000          	// #429496729600
  8c:	d2c00c82 	mov	x2, #0x6400000000          	// #429496729600
  90:	d2e00c82 	mov	x2, #0x64000000000000      	// #28147497671065600
  94:	d2e00c82 	mov	x2, #0x64000000000000      	// #28147497671065600
  98:	52800c81 	mov	w1, #0x64                  	// #100
  9c:	52800c81 	mov	w1, #0x64                  	// #100
  a0:	52a00c81 	mov	w1, #0x640000              	// #6553600
  a4:	8a030041 	and	x1, x2, x3
  a8:	0a0f015e 	and	w30, w10, w15
  ac:	12000041 	and	w1, w2, #0x1
  b0:	8a430441 	and	x1, x2, x3, lsr #1
  b4:	32000021 	orr	w1, w1, #0x1
  b8:	32000021 	orr	w1, w1, #0x1
  bc:	b2400021 	orr	x1, x1, #0x1
  c0:	92400c41 	and	x1, x2, #0xf
  c4:	12000c41 	and	w1, w2, #0xf
  c8:	92610041 	and	x1, x2, #0x80000000
  cc:	12010041 	and	w1, w2, #0x80000000
  d0:	925d0041 	and	x1, x2, #0x800000000
  d4:	92400c85 	and	x5, x4, #0xf
  d8:	0a230041 	bic	w1, w2, w3
  dc:	8a230041 	bic	x1, x2, x3
  e0:	54000001 	b\.ne	e0 <sp\+0x90>  // b\.any
  e4:	17ffffff 	b	e0 <sp\+0x90>
  e8:	14000001 	b	ec <sp\+0x9c>
  ec:	54ffffa0 	b\.eq	e0 <sp\+0x90>  // b\.none
  f0:	54000001 	b\.ne	f0 <sp\+0xa0>  // b\.any
  f4:	17ffffff 	b	f0 <sp\+0xa0>
  f8:	14000001 	b	fc <sp\+0xac>
  fc:	54ffffa0 	b\.eq	f0 <sp\+0xa0>  // b\.none
 100:	d61f0040 	br	x2
 104:	54ffffc2 	b\.cs	fc <sp\+0xac>  // b\.hs, b\.nlast
 108:	54ffffa3 	b\.cc	fc <sp\+0xac>  // b\.lo, b\.ul, b\.last
	...
			10c: R_AARCH64_ABS32	.text\+0x50
			110: R_AARCH64_ABS64	.text\+0x50
