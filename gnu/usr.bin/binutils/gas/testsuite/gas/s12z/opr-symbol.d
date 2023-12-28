#objdump: -dt
#name:    OPR addressing mode: symbols in its direct submode
#source:  opr-symbol.s

.*:     file format elf32-s12z

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00123456 l       \*ABS\*	00000000 sym1
00abcdef l       \*ABS\*	00000000 sym2



Disassembly of section .text:

00000000 <.text>:
   0:	0c 17 fa 12 	mov.b #23, sym1
   4:	34 56 
   6:	0d 00 17 fa 	mov.w #23, sym1
   a:	12 34 56 
   d:	0e 00 00 17 	mov.p #23, sym1
  11:	fa 12 34 56 
  15:	0f 00 00 00 	mov.l #23, sym1
  19:	17 fa 12 34 
  1d:	56 
  1e:	1c fa 12 34 	mov.b sym1, sym2
  22:	56 fa ab cd 
  26:	ef 
  27:	1d fa 12 34 	mov.w sym1, sym2
  2b:	56 fa ab cd 
  2f:	ef 
  30:	1e fa 12 34 	mov.p sym1, sym2
  34:	56 fa ab cd 
  38:	ef 
  39:	1f fa 12 34 	mov.l sym1, sym2
  3d:	56 fa ab cd 
  41:	ef 
  42:	65 fa 12 34 	add d1, sym1
  46:	56 
  47:	68 fa 12 34 	and d2, sym1
  4b:	56 
  4c:	81 fa 12 34 	sub d3, sym1
  50:	56 
  51:	8a fa 12 34 	or d4, sym1
  55:	56 
  56:	9c fa 12 34 	inc.b sym1
  5a:	56 
  5b:	9d fa 12 34 	inc.w sym1
  5f:	56 
  60:	9f fa 12 34 	inc.l sym1
  64:	56 
  65:	b2 12 34 56 	ld d4, sym1
  69:	ba ab cd ef 	jmp sym2
  6d:	bb 12 34 56 	jsr sym1
  71:	ac fa 12 34 	dec.b sym1
  75:	56 
  76:	ad fa 12 34 	dec.w sym1
  7a:	56 
  7b:	af fa 12 34 	dec.l sym1
  7f:	56 
  80:	bc fa 12 34 	clr.b sym1
  84:	56 
  85:	bd fa 12 34 	clr.w sym1
  89:	56 
  8a:	bf fa 12 34 	clr.l sym1
  8e:	56 
  8f:	d3 12 34 56 	st d5, sym1
  93:	cc fa 12 34 	com.b sym1
  97:	56 
  98:	cd fa 12 34 	com.w sym1
  9c:	56 
  9d:	cf fa 12 34 	com.l sym1
  a1:	56 
  a2:	dc fa 12 34 	neg.b sym1
  a6:	56 
  a7:	dd fa 12 34 	neg.w sym1
  ab:	56 
  ac:	df fa 12 34 	neg.l sym1
  b0:	56 
  b1:	f3 fa 12 34 	cmp d5, sym1
  b5:	56 
  b6:	1b 00 fa 12 	ld s, sym1
  ba:	34 56 
  bc:	1b 01 fa 12 	st s, sym1
  c0:	34 56 
  c2:	1b 02 fa 12 	cmp s, sym1
  c6:	34 56 
  c8:	1b 10 fa 12 	minu d2, sym1
  cc:	34 56 
  ce:	1b 18 fa 12 	maxu d2, sym1
  d2:	34 56 
  d4:	1b 20 fa 12 	mins d2, sym1
  d8:	34 56 
  da:	1b 28 fa 12 	maxs d2, sym1
  de:	34 56 
  e0:	1b 61 fa 12 	adc d3, sym1
  e4:	34 56 
  e6:	1b 69 fa 12 	bit d3, sym1
  ea:	34 56 
  ec:	1b 81 fa 12 	sbc d3, sym1
  f0:	34 56 
  f2:	1b 89 fa 12 	eor d3, sym1
  f6:	34 56 
  f8:	02 d1 fa 12 	brclr.b sym1, d1, \*\+3
  fc:	34 56 03 
  ff:	02 a2 fa 12 	brclr.w sym1, #2, \*\+4
 103:	34 56 04 
 106:	03 81 fa ab 	brset.b sym2, d2, \*\+5
 10a:	cd ef 05 
 10d:	03 b2 fa ab 	brset.w sym2, #3, \*\+6
 111:	cd ef 06 
 114:	4d 40 fa 12 	mulu.b d1, d2, sym1
 118:	34 56 
 11a:	4e 7e fa 12 	mulu.ll d6, sym1, sym2
 11e:	34 56 fa ab 
 122:	cd ef 
 124:	4d c0 fa 12 	muls.b d1, d2, sym1
 128:	34 56 
 12a:	4e fe fa 12 	muls.ll d6, sym1, sym2
 12e:	34 56 fa ab 
 132:	cd ef 
 134:	1b b5 c0 fa 	qmuls.b d1, d2, sym1
 138:	12 34 56 
 13b:	1b b6 fe fa 	qmuls.ll d6, sym1, sym2
 13f:	12 34 56 fa 
 143:	ab cd ef 
 146:	1b 35 40 fa 	divu.b d1, d2, sym1
 14a:	12 34 56 
 14d:	1b 36 7e fa 	divu.ll d6, sym1, sym2
 151:	12 34 56 fa 
 155:	ab cd ef 
 158:	1b 35 c0 fa 	divs.b d1, d2, sym1
 15c:	12 34 56 
 15f:	1b 36 fe fa 	divs.ll d6, sym1, sym2
 163:	12 34 56 fa 
 167:	ab cd ef 
 16a:	ec a0 fa 12 	bclr.b sym1, #2
 16e:	34 56 
 170:	ec 85 fa ab 	bclr.w sym2, d2
 174:	cd ef 
 176:	ed a0 fa 12 	bset.b sym1, #2
 17a:	34 56 
 17c:	ed 85 fa ab 	bset.w sym2, d2
 180:	cd ef 
 182:	ee a0 fa 12 	btgl.b sym1, #2
 186:	34 56 
 188:	ee 85 fa ab 	btgl.w sym2, d2
 18c:	cd ef 
 18e:	0b 0c fa 12 	tbne.b sym1, \*\+8
 192:	34 56 08 
 195:	0b ad fa ab 	dbpl.w sym2, \*\+9
 199:	cd ef 09 
 19c:	1b 08 c0 fa 	bfins.b d2, sym1, d2
 1a0:	12 34 56 
 1a3:	1b 08 d4 fa 	bfins.w sym1, d2, d2
 1a7:	12 34 56 
 1aa:	1b 08 e9 01 	bfins.p d2, sym1, #8\:1
 1ae:	fa 12 34 56 
 1b2:	1b 08 fd 01 	bfins.l sym1, d2, #8\:1
 1b6:	fa 12 34 56 
 1ba:	1b 08 40 fa 	bfext.b d2, sym1, d2
 1be:	12 34 56 
 1c1:	1b 08 54 fa 	bfext.w sym1, d2, d2
 1c5:	12 34 56 
 1c8:	1b 08 69 01 	bfext.p d2, sym1, #8\:1
 1cc:	fa 12 34 56 
 1d0:	1b 08 7c e2 	bfext.l sym1, d2, #7\:2
 1d4:	fa 12 34 56 
