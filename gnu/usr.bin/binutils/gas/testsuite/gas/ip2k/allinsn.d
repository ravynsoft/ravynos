#as:
#objdump: -dr
#name: allinsn

.*: +file format .*


Disassembly of section \.text:

00000000 <jmp>:
   0:	e0 01       	jmp \$00002
   2:	f0 00       	jmp \$02000
   4:	e8 00       	jmp \$01000
   6:	e7 ff       	jmp \$00ffe
   8:	e0 01       	jmp \$00002
   a:	e5 c8       	jmp \$00b90
   c:	e4 28       	jmp \$00850
   e:	e5 b7       	jmp \$00b6e

00000010 <call>:
  10:	c0 02       	call \$00004
  12:	d0 00       	call \$02000
  14:	c8 00       	call \$01000
  16:	c7 ff       	call \$00ffe
  18:	c0 01       	call \$00002
  1a:	ce 6c       	call \$01cd8
  1c:	cf 9f       	call \$01f3e
  1e:	c9 e9       	call \$013d2

00000020 <sb>:
  20:	b2 01       	sb \$01,1
  22:	be 19       	sb INTED,7
  24:	b8 19       	sb INTED,4
  26:	b6 19       	sb INTED,3
  28:	b2 01       	sb \$01,1
  2a:	be 18       	sb INTE,7
  2c:	b2 10       	sb ADDRH,1
  2e:	bc 0c       	sb DPH,6

00000030 <snb>:
  30:	a2 01       	snb \$01,1
  32:	ae 0b       	snb STATUS,7
  34:	a8 38       	snb \$38,4
  36:	a6 19       	snb INTED,3
  38:	a2 01       	snb \$01,1
  3a:	aa 29       	snb RCOUT,5
  3c:	a2 3e       	snb \$3e,1
  3e:	a2 2b       	snb LFSRA,1

00000040 <setb>:
  40:	92 01       	setb \$01,1
  42:	9e 0b       	setb STATUS,7
  44:	98 38       	setb \$38,4
  46:	96 19       	setb INTED,3
  48:	92 01       	setb \$01,1
  4a:	92 17       	setb INTF,1
  4c:	9c 19       	setb INTED,6
  4e:	96 1c       	setb XCFG,3

00000050 <clrb>:
  50:	82 01       	clrb \$01,1
  52:	8e 0b       	clrb STATUS,7
  54:	88 38       	clrb \$38,4
  56:	86 19       	clrb INTED,3
  58:	82 01       	clrb \$01,1
  5a:	8e 24       	clrb RBIN,7
  5c:	86 0f       	clrb MULH,3
  5e:	8a 12       	clrb DATAH,5

00000060 <xorw_l>:
  60:	7f 00       	xor W,#\$00
  62:	7f 19       	xor W,#\$19
  64:	7f 0c       	xor W,#\$0c
  66:	7f 7b       	xor W,#\$7b
  68:	7f 01       	xor W,#\$01
  6a:	7f 14       	xor W,#\$14
  6c:	7f 7a       	xor W,#\$7a
  6e:	7f 0f       	xor W,#\$0f

00000070 <andw_l>:
  70:	7e 00       	and W,#\$00
  72:	7e 19       	and W,#\$19
  74:	7e 0c       	and W,#\$0c
  76:	7e 0c       	and W,#\$0c
  78:	7e 01       	and W,#\$01
  7a:	7e 12       	and W,#\$12
  7c:	7e 1d       	and W,#\$1d
  7e:	7e 0e       	and W,#\$0e

00000080 <orw_l>:
  80:	7d 00       	or W,#\$00
  82:	7d 19       	or W,#\$19
  84:	7d 0c       	or W,#\$0c
  86:	7d 0c       	or W,#\$0c
  88:	7d 01       	or W,#\$01
  8a:	7d 20       	or W,#\$20
  8c:	7d 0e       	or W,#\$0e
  8e:	7d 21       	or W,#\$21

00000090 <addw_l>:
  90:	7b 00       	add W,#\$00
  92:	7b 19       	add W,#\$19
  94:	7b 0c       	add W,#\$0c
  96:	7b 0c       	add W,#\$0c
  98:	7b 01       	add W,#\$01
  9a:	7b 15       	add W,#\$15
  9c:	7b 18       	add W,#\$18
  9e:	7b 2f       	add W,#\$2f

000000a0 <subw_l>:
  a0:	7a 00       	sub W,#\$00
  a2:	7a 19       	sub W,#\$19
  a4:	7a d4       	sub W,#\$d4
  a6:	7a 0c       	sub W,#\$0c
  a8:	7a 01       	sub W,#\$01
  aa:	7a 70       	sub W,#\$70
  ac:	7a 54       	sub W,#\$54
  ae:	7a e1       	sub W,#\$e1

000000b0 <cmpw_l>:
  b0:	79 00       	cmp W,#\$00
  b2:	79 19       	cmp W,#\$19
  b4:	79 0c       	cmp W,#\$0c
  b6:	79 0c       	cmp W,#\$0c
  b8:	79 01       	cmp W,#\$01
  ba:	79 0b       	cmp W,#\$0b
  bc:	79 0d       	cmp W,#\$0d
  be:	79 13       	cmp W,#\$13

000000c0 <retw_l>:
  c0:	78 00       	retw #\$00
  c2:	78 19       	retw #\$19
  c4:	78 7a       	retw #\$7a
  c6:	78 0c       	retw #\$0c
  c8:	78 01       	retw #\$01
  ca:	78 c9       	retw #\$c9
  cc:	78 0e       	retw #\$0e
  ce:	78 14       	retw #\$14

000000d0 <csew_l>:
  d0:	77 00       	cse W,#\$00
  d2:	77 19       	cse W,#\$19
  d4:	77 79       	cse W,#\$79
  d6:	77 7a       	cse W,#\$7a
  d8:	77 01       	cse W,#\$01
  da:	77 0c       	cse W,#\$0c
  dc:	77 e7       	cse W,#\$e7
  de:	77 15       	cse W,#\$15

000000e0 <csnew_l>:
  e0:	76 00       	csne W,#\$00
  e2:	76 19       	csne W,#\$19
  e4:	76 7a       	csne W,#\$7a
  e6:	76 0c       	csne W,#\$0c
  e8:	76 01       	csne W,#\$01
  ea:	76 16       	csne W,#\$16
  ec:	76 70       	csne W,#\$70
  ee:	76 16       	csne W,#\$16

000000f0 <push_l>:
  f0:	74 00       	push #\$00
  f2:	74 19       	push #\$19
  f4:	74 70       	push #\$70
  f6:	74 0c       	push #\$0c
  f8:	74 01       	push #\$01
  fa:	74 12       	push #\$12
  fc:	74 0f       	push #\$0f
  fe:	74 7a       	push #\$7a

00000100 <mulsw_l>:
 100:	73 00       	muls W,#\$00
 102:	73 19       	muls W,#\$19
 104:	73 0c       	muls W,#\$0c
 106:	73 0c       	muls W,#\$0c
 108:	73 01       	muls W,#\$01
 10a:	73 17       	muls W,#\$17
 10c:	73 15       	muls W,#\$15
 10e:	73 12       	muls W,#\$12

00000110 <muluw_l>:
 110:	72 00       	mulu W,#\$00
 112:	72 19       	mulu W,#\$19
 114:	72 0c       	mulu W,#\$0c
 116:	72 0c       	mulu W,#\$0c
 118:	72 01       	mulu W,#\$01
 11a:	72 0f       	mulu W,#\$0f
 11c:	72 15       	mulu W,#\$15
 11e:	72 17       	mulu W,#\$17

00000120 <loadl_l>:
 120:	71 00       	loadl #\$00
 122:	71 19       	loadl #\$19
 124:	71 0c       	loadl #\$0c
 126:	71 0c       	loadl #\$0c
 128:	71 01       	loadl #\$01
 12a:	71 10       	loadl #\$10
 12c:	71 10       	loadl #\$10
 12e:	71 15       	loadl #\$15

00000130 <loadh_l>:
 130:	70 00       	loadh #\$00
 132:	70 19       	loadh #\$19
 134:	70 0c       	loadh #\$0c
 136:	70 0c       	loadh #\$0c
 138:	70 01       	loadh #\$01
 13a:	70 11       	loadh #\$11
 13c:	70 18       	loadh #\$18
 13e:	70 18       	loadh #\$18

00000140 <loadl_a>:
 140:	71 01       	loadl #\$01
 142:	71 19       	loadl #\$19
 144:	71 0c       	loadl #\$0c
 146:	71 0c       	loadl #\$0c
 148:	71 01       	loadl #\$01
 14a:	71 4c       	loadl #\$4c
 14c:	71 14       	loadl #\$14
 14e:	71 34       	loadl #\$34

00000150 <loadh_a>:
 150:	70 00       	loadh #\$00
 152:	70 00       	loadh #\$00
 154:	70 00       	loadh #\$00
 156:	70 00       	loadh #\$00
 158:	70 00       	loadh #\$00
 15a:	70 00       	loadh #\$00
 15c:	70 00       	loadh #\$00
 15e:	70 00       	loadh #\$00

00000160 <addcfr_w>:
 160:	5e 01       	addc \$01,W
 162:	5e 0b       	addc STATUS,W
 164:	5e 38       	addc \$38,W
 166:	5e 19       	addc INTED,W
 168:	5e 64       	addc S1TCFG,W
 16a:	5e 22       	addc RADIR,W
 16c:	5e 32       	addc REDIR,W
 16e:	5e 18       	addc INTE,W

00000170 <addcw_fr>:
 170:	5c 01       	addc W,\$01
 172:	5c 0b       	addc W,STATUS
 174:	5c 1a       	addc W,FCFG
 176:	5c 19       	addc W,INTED
 178:	5c 0a       	addc W,WREG
 17a:	5c 1b       	addc W,TCTRL
 17c:	5c 6f       	addc W,CMPCFG
 17e:	5c 16       	addc W,INTSPD

00000180 <incsnz_fr>:
 180:	5a 03       	incsnz ADDRX
 182:	5a 0b       	incsnz STATUS
 184:	5a 38       	incsnz \$38
 186:	5a 19       	incsnz INTED
 188:	5a 01       	incsnz \$01
 18a:	5a 32       	incsnz REDIR
 18c:	5a 25       	incsnz RBOUT
 18e:	5a 2b       	incsnz LFSRA

00000190 <incsnzw_fr>:
 190:	58 01       	incsnz W,\$01
 192:	58 0b       	incsnz W,STATUS
 194:	58 1a       	incsnz W,FCFG
 196:	58 19       	incsnz W,INTED
 198:	58 01       	incsnz W,\$01
 19a:	58 21       	incsnz W,RAOUT
 19c:	58 1d       	incsnz W,EMCFG
 19e:	58 18       	incsnz W,INTE

000001a0 <mulsw_fr>:
 1a0:	54 01       	muls W,\$01
 1a2:	54 0b       	muls W,STATUS
 1a4:	54 1a       	muls W,FCFG
 1a6:	54 19       	muls W,INTED
 1a8:	54 01       	muls W,\$01
 1aa:	54 17       	muls W,INTF
 1ac:	54 0d       	muls W,DPL
 1ae:	54 25       	muls W,RBOUT

000001b0 <muluw_fr>:
 1b0:	50 01       	mulu W,\$01
 1b2:	50 0b       	mulu W,STATUS
 1b4:	50 1a       	mulu W,FCFG
 1b6:	50 19       	mulu W,INTED
 1b8:	50 01       	mulu W,\$01
 1ba:	50 15       	mulu W,INTVECL
 1bc:	50 15       	mulu W,INTVECL
 1be:	50 22       	mulu W,RADIR

000001c0 <decsnz_fr>:
 1c0:	4e 01       	decsnz \$01
 1c2:	4e 0b       	decsnz STATUS
 1c4:	4e 38       	decsnz \$38
 1c6:	4e 19       	decsnz INTED
 1c8:	4e 01       	decsnz \$01
 1ca:	4e 2b       	decsnz LFSRA
 1cc:	4e 06       	decsnz SPH
 1ce:	4e 1e       	decsnz IPCH

000001d0 <decsnzw_fr>:
 1d0:	4c 01       	decsnz W,\$01
 1d2:	4c 0b       	decsnz W,STATUS
 1d4:	4c 1a       	decsnz W,FCFG
 1d6:	4c 19       	decsnz W,INTED
 1d8:	4c 01       	decsnz W,\$01
 1da:	4c 18       	decsnz W,INTE
 1dc:	4c 3a       	decsnz W,RGDIR
 1de:	4c 14       	decsnz W,INTVECH

000001e0 <subcw_fr>:
 1e0:	48 01       	subc W,\$01
 1e2:	48 0b       	subc W,STATUS
 1e4:	48 1a       	subc W,FCFG
 1e6:	48 19       	subc W,INTED
 1e8:	48 01       	subc W,\$01
 1ea:	48 2b       	subc W,LFSRA
 1ec:	48 0d       	subc W,DPL
 1ee:	48 21       	subc W,RAOUT

000001f0 <subcfr_w>:
 1f0:	4a 01       	subc \$01,W
 1f2:	4a 0b       	subc STATUS,W
 1f4:	4a 38       	subc \$38,W
 1f6:	4a 19       	subc INTED,W
 1f8:	4a 01       	subc \$01,W
 1fa:	4a 0f       	subc MULH,W
 1fc:	4a 15       	subc INTVECL,W
 1fe:	4a 2b       	subc LFSRA,W

00000200 <pop_fr>:
 200:	46 01       	pop \$01
 202:	46 0b       	pop STATUS
 204:	46 38       	pop \$38
 206:	46 19       	pop INTED
 208:	46 01       	pop \$01
 20a:	46 23       	pop LFSRH
 20c:	46 0a       	pop WREG
 20e:	46 0d       	pop DPL

00000210 <push_fr>:
 210:	44 01       	push \$01
 212:	44 0b       	push STATUS
 214:	44 38       	push \$38
 216:	44 19       	push INTED
 218:	44 01       	push \$01
 21a:	44 1a       	push FCFG
 21c:	44 0d       	push DPL
 21e:	44 0d       	push DPL

00000220 <csew_fr>:
 220:	42 01       	cse W,\$01
 222:	42 0b       	cse W,STATUS
 224:	42 1a       	cse W,FCFG
 226:	42 19       	cse W,INTED
 228:	42 01       	cse W,\$01
 22a:	42 1b       	cse W,TCTRL
 22c:	42 0f       	cse W,MULH
 22e:	42 57       	cse W,T2CAP1L

00000230 <csnew_fr>:
 230:	40 02       	csne W,ADDRSEL
 232:	40 0b       	csne W,STATUS
 234:	40 1a       	csne W,FCFG
 236:	40 19       	csne W,INTED
 238:	40 01       	csne W,\$01
 23a:	40 27       	csne W,LFSRL
 23c:	40 11       	csne W,ADDRL
 23e:	40 2b       	csne W,LFSRA

00000240 <incsz_fr>:
 240:	3e 01       	incsz \$01
 242:	3e 0b       	incsz STATUS
 244:	3e 38       	incsz \$38
 246:	3e 19       	incsz INTED
 248:	3e 01       	incsz \$01
 24a:	3e 2d       	incsz RDOUT
 24c:	3e 18       	incsz INTE
 24e:	3e 4d       	incsz T1CFG1L

00000250 <incszw_fr>:
 250:	3c 01       	incsz W,\$01
 252:	3c 0b       	incsz W,STATUS
 254:	3c 1a       	incsz W,FCFG
 256:	3c 19       	incsz W,INTED
 258:	3c 01       	incsz W,\$01
 25a:	3c 4d       	incsz W,T1CFG1L
 25c:	3c 0b       	incsz W,STATUS
 25e:	3c 62       	incsz W,S1TBUFH

00000260 <swap_fr>:
 260:	3a 01       	swap \$01
 262:	3a 0b       	swap STATUS
 264:	3a 38       	swap \$38
 266:	3a 19       	swap INTED
 268:	3a 02       	swap ADDRSEL
 26a:	3a 21       	swap RAOUT
 26c:	3a 18       	swap INTE
 26e:	3a 33       	swap \$33

00000270 <swapw_fr>:
 270:	38 01       	swap W,\$01
 272:	38 0b       	swap W,STATUS
 274:	38 1a       	swap W,FCFG
 276:	38 19       	swap W,INTED
 278:	38 01       	swap W,\$01
 27a:	38 2b       	swap W,LFSRA
 27c:	38 20       	swap W,RAIN
 27e:	38 11       	swap W,ADDRL

00000280 <rl_fr>:
 280:	36 02       	rl ADDRSEL
 282:	36 0b       	rl STATUS
 284:	36 38       	rl \$38
 286:	36 19       	rl INTED
 288:	36 01       	rl \$01
 28a:	36 1e       	rl IPCH
 28c:	36 22       	rl RADIR
 28e:	36 2b       	rl LFSRA

00000290 <rlw_fr>:
 290:	34 02       	rl W,ADDRSEL
 292:	34 0b       	rl W,STATUS
 294:	34 1a       	rl W,FCFG
 296:	34 19       	rl W,INTED
 298:	34 01       	rl W,\$01
 29a:	34 0e       	rl W,SPDREG
 29c:	34 18       	rl W,INTE
 29e:	34 1b       	rl W,TCTRL

000002a0 <rr_fr>:
 2a0:	32 01       	rr \$01
 2a2:	32 0b       	rr STATUS
 2a4:	32 38       	rr \$38
 2a6:	32 19       	rr INTED
 2a8:	32 01       	rr \$01
 2aa:	32 2b       	rr LFSRA
 2ac:	32 19       	rr INTED
 2ae:	32 10       	rr ADDRH

000002b0 <rrw_fr>:
 2b0:	30 01       	rr W,\$01
 2b2:	30 0b       	rr W,STATUS
 2b4:	30 1a       	rr W,FCFG
 2b6:	30 19       	rr W,INTED
 2b8:	30 01       	rr W,\$01
 2ba:	30 10       	rr W,ADDRH
 2bc:	30 48       	rr W,T1CAP2H
 2be:	30 11       	rr W,ADDRL

000002c0 <decsz_fr>:
 2c0:	2e 01       	decsz \$01
 2c2:	2e 0b       	decsz STATUS
 2c4:	2e 38       	decsz \$38
 2c6:	2e 19       	decsz INTED
 2c8:	2e 01       	decsz \$01
 2ca:	2e 4e       	decsz T1CFG2H
 2cc:	2e 1d       	decsz EMCFG
 2ce:	2e 10       	decsz ADDRH

000002d0 <decszw_fr>:
 2d0:	2c 01       	decsz W,\$01
 2d2:	2c 0b       	decsz W,STATUS
 2d4:	2c 1a       	decsz W,FCFG
 2d6:	2c 19       	decsz W,INTED
 2d8:	2c 01       	decsz W,\$01
 2da:	2c 1a       	decsz W,FCFG
 2dc:	2c 16       	decsz W,INTSPD
 2de:	2c 04       	decsz W,IPH

000002e0 <inc_fr>:
 2e0:	2a 01       	inc \$01
 2e2:	2a 0b       	inc STATUS
 2e4:	2a 38       	inc \$38
 2e6:	2a 19       	inc INTED
 2e8:	2a 01       	inc \$01
 2ea:	2a 2b       	inc LFSRA
 2ec:	2a 2b       	inc LFSRA
 2ee:	2a 53       	inc ADCTMR

000002f0 <incw_fr>:
 2f0:	28 01       	inc W,\$01
 2f2:	28 0b       	inc W,STATUS
 2f4:	28 1a       	inc W,FCFG
 2f6:	28 19       	inc W,INTED
 2f8:	28 01       	inc W,\$01
 2fa:	28 2b       	inc W,LFSRA
 2fc:	28 1e       	inc W,IPCH
 2fe:	28 21       	inc W,RAOUT

00000300 <not_fr>:
 300:	26 01       	not \$01
 302:	26 0b       	not STATUS
 304:	26 38       	not \$38
 306:	26 19       	not INTED
 308:	26 01       	not \$01
 30a:	26 2b       	not LFSRA
 30c:	26 0e       	not SPDREG
 30e:	26 2b       	not LFSRA

00000310 <notw_fr>:
 310:	24 01       	not W,\$01
 312:	24 0b       	not W,STATUS
 314:	24 1a       	not W,FCFG
 316:	24 19       	not W,INTED
 318:	24 01       	not W,\$01
 31a:	24 54       	not W,T2CNTH
 31c:	24 2b       	not W,LFSRA
 31e:	24 32       	not W,REDIR

00000320 <test_fr>:
 320:	22 02       	test ADDRSEL
 322:	22 0b       	test STATUS
 324:	22 38       	test \$38
 326:	22 d7       	test \$d7
 328:	22 01       	test \$01
 32a:	22 2b       	test LFSRA
 32c:	22 18       	test INTE
 32e:	22 19       	test INTED

00000330 <movw_l>:
 330:	7c 00       	mov W,#\$00
 332:	7c 19       	mov W,#\$19
 334:	7c 0c       	mov W,#\$0c
 336:	7c 0c       	mov W,#\$0c
 338:	7c 01       	mov W,#\$01
 33a:	7c 0e       	mov W,#\$0e
 33c:	7c 0b       	mov W,#\$0b
 33e:	7c 42       	mov W,#\$42

00000340 <movfr_w>:
 340:	02 01       	mov \$01,W
 342:	02 0b       	mov STATUS,W
 344:	02 38       	mov \$38,W
 346:	02 19       	mov INTED,W
 348:	02 01       	mov \$01,W
 34a:	02 24       	mov RBIN,W
 34c:	02 56       	mov T2CAP1H,W
 34e:	02 12       	mov DATAH,W

00000350 <movw_fr>:
 350:	20 01       	mov W,\$01
 352:	20 0b       	mov W,STATUS
 354:	20 1a       	mov W,FCFG
 356:	20 19       	mov W,INTED
 358:	20 01       	mov W,\$01
 35a:	20 0c       	mov W,DPH
 35c:	20 2b       	mov W,LFSRA
 35e:	20 17       	mov W,INTF

00000360 <addfr_w>:
 360:	1e 0a       	add WREG,W
 362:	1e 0b       	add STATUS,W
 364:	1e 38       	add \$38,W
 366:	1e d7       	add \$d7,W
 368:	1e 01       	add \$01,W
 36a:	1e 2b       	add LFSRA,W
 36c:	1e 19       	add INTED,W
 36e:	1e 27       	add LFSRL,W

00000370 <addw_fr>:
 370:	1c 01       	add W,\$01
 372:	1c 0b       	add W,STATUS
 374:	1c 1a       	add W,FCFG
 376:	1c 19       	add W,INTED
 378:	1c 01       	add W,\$01
 37a:	1c 13       	add W,DATAL
 37c:	1c 5b       	add W,T2CMP1L
 37e:	1c 19       	add W,INTED

00000380 <xorfr_w>:
 380:	1a 01       	xor \$01,W
 382:	1a 0b       	xor STATUS,W
 384:	1a 38       	xor \$38,W
 386:	1a 19       	xor INTED,W
 388:	1a 02       	xor ADDRSEL,W
 38a:	1a 1f       	xor IPCL,W
 38c:	1a 16       	xor INTSPD,W
 38e:	1a 2b       	xor LFSRA,W

00000390 <xorw_fr>:
 390:	18 02       	xor W,ADDRSEL
 392:	18 0b       	xor W,STATUS
 394:	18 1a       	xor W,FCFG
 396:	18 19       	xor W,INTED
 398:	18 01       	xor W,\$01
 39a:	18 0e       	xor W,SPDREG
 39c:	18 0a       	xor W,WREG
 39e:	18 15       	xor W,INTVECL

000003a0 <andfr_w>:
 3a0:	16 01       	and \$01,W
 3a2:	16 0b       	and STATUS,W
 3a4:	16 38       	and \$38,W
 3a6:	16 19       	and INTED,W
 3a8:	16 01       	and \$01,W
 3aa:	16 1c       	and XCFG,W
 3ac:	16 25       	and RBOUT,W
 3ae:	16 18       	and INTE,W

000003b0 <andw_fr>:
 3b0:	14 01       	and W,\$01
 3b2:	14 0b       	and W,STATUS
 3b4:	14 1a       	and W,FCFG
 3b6:	14 19       	and W,INTED
 3b8:	14 01       	and W,\$01
 3ba:	14 15       	and W,INTVECL
 3bc:	14 28       	and W,RCIN
 3be:	14 2b       	and W,LFSRA

000003c0 <orfr_w>:
 3c0:	12 01       	or \$01,W
 3c2:	12 0b       	or STATUS,W
 3c4:	12 38       	or \$38,W
 3c6:	12 19       	or INTED,W
 3c8:	12 01       	or \$01,W
 3ca:	12 3a       	or RGDIR,W
 3cc:	12 1d       	or EMCFG,W
 3ce:	12 0a       	or WREG,W

000003d0 <orw_fr>:
 3d0:	10 01       	or W,\$01
 3d2:	10 0b       	or W,STATUS
 3d4:	10 1a       	or W,FCFG
 3d6:	10 19       	or W,INTED
 3d8:	10 01       	or W,\$01
 3da:	10 0b       	or W,STATUS
 3dc:	10 18       	or W,INTE
 3de:	10 3b       	or W,\$3b

000003e0 <dec_fr>:
 3e0:	0e 02       	dec ADDRSEL
 3e2:	0e 33       	dec \$33
 3e4:	0e 1a       	dec FCFG
 3e6:	0e 19       	dec INTED
 3e8:	0e 01       	dec \$01
 3ea:	0e 4c       	dec T1CFG1H
 3ec:	0e 20       	dec RAIN
 3ee:	0e 11       	dec ADDRL

000003f0 <decw_fr>:
 3f0:	0c 02       	dec W,ADDRSEL
 3f2:	0c 33       	dec W,\$33
 3f4:	0c 38       	dec W,\$38
 3f6:	0c 19       	dec W,INTED
 3f8:	0c 01       	dec W,\$01
 3fa:	0c 01       	dec W,\$01
 3fc:	0c 44       	dec W,T1CNTH
 3fe:	0c 07       	dec W,SPL

00000400 <subfr_w>:
 400:	0a 02       	sub ADDRSEL,W
 402:	0a 0b       	sub STATUS,W
 404:	0a 0f       	sub MULH,W
 406:	0a 19       	sub INTED,W
 408:	0a 01       	sub \$01,W
 40a:	0a 28       	sub RCIN,W
 40c:	0a 37       	sub \$37,W
 40e:	0a 11       	sub ADDRL,W

00000410 <subw_fr>:
 410:	08 01       	sub W,\$01
 412:	08 15       	sub W,INTVECL
 414:	08 19       	sub W,INTED
 416:	08 19       	sub W,INTED
 418:	08 01       	sub W,\$01
 41a:	08 11       	sub W,ADDRL
 41c:	08 10       	sub W,ADDRH
 41e:	08 12       	sub W,DATAH

00000420 <clr_fr>:
 420:	06 0a       	clr WREG
 422:	06 0b       	clr STATUS
 424:	06 19       	clr INTED
 426:	06 19       	clr INTED
 428:	06 01       	clr \$01
 42a:	06 18       	clr INTE
 42c:	06 d7       	clr \$d7
 42e:	06 17       	clr INTF

00000430 <cmpw_fr>:
 430:	04 01       	cmp W,\$01
 432:	04 15       	cmp W,INTVECL
 434:	04 19       	cmp W,INTED
 436:	04 19       	cmp W,INTED
 438:	04 01       	cmp W,\$01
 43a:	04 12       	cmp W,DATAH
 43c:	04 14       	cmp W,INTVECH
 43e:	04 10       	cmp W,ADDRH

00000440 <speed>:
 440:	01 00       	speed #\$00
 442:	01 19       	speed #\$19
 444:	01 0c       	speed #\$0c
 446:	01 0c       	speed #\$0c
 448:	01 01       	speed #\$01
 44a:	01 0e       	speed #\$0e
 44c:	01 12       	speed #\$12
 44e:	01 61       	speed #\$61

00000450 <ireadi>:
 450:	00 1d       	ireadi

00000452 <iwritei>:
 452:	00 1c       	iwritei

00000454 <fread>:
 454:	00 1b       	fread

00000456 <fwrite>:
 456:	00 1a       	fwrite

00000458 <iread>:
 458:	00 19       	iread

0000045a <iwrite>:
 45a:	00 18       	iwrite

0000045c <page>:
 45c:	00 10       	page \$00000
 45e:	00 10       	page \$00000
 460:	00 10       	page \$00000
 462:	00 10       	page \$00000
 464:	00 10       	page \$00000
 466:	00 10       	page \$00000
 468:	00 10       	page \$00000
 46a:	00 10       	page \$00000

0000046c <system>:
 46c:	00 ff       	system

0000046e <reti>:
 46e:	00 08       	reti #\$0
 470:	00 09       	reti #\$1
 472:	00 0a       	reti #\$2
 474:	00 0b       	reti #\$3
 476:	00 0c       	reti #\$4
 478:	00 0d       	reti #\$5
 47a:	00 0e       	reti #\$6
 47c:	00 0f       	reti #\$7

0000047e <ret>:
 47e:	00 07       	ret

00000480 <int>:
 480:	00 06       	int

00000482 <breakx>:
 482:	00 05       	breakx

00000484 <cwdt>:
 484:	00 04       	cwdt

00000486 <ferase>:
 486:	00 03       	ferase

00000488 <retnp>:
 488:	00 02       	retnp

0000048a <break>:
 48a:	00 01       	break

0000048c <nop>:
	\.\.\.
