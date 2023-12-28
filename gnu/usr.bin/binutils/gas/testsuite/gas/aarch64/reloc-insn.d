#as: -mabi=lp64
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	d281ffe0 	mov	x0, #0xfff                 	// #4095
   4:	9280ffe0 	mov	x0, #0xfffffffffffff800    	// #-2048
   8:	d2a24681 	mov	x1, #0x12340000            	// #305397760
   c:	f28acf01 	movk	x1, #0x5678
  10:	92a00001 	movn	x1, #0x0, lsl #16
  14:	f29f0001 	movk	x1, #0xf800
  18:	d2d55761 	mov	x1, #0xaabb00000000        	// #187720135606272
  1c:	f2b99ba1 	movk	x1, #0xccdd, lsl #16
  20:	f29ddfe1 	movk	x1, #0xeeff
  24:	d2c00001 	movz	x1, #0x0, lsl #32
			24: R_AARCH64_MOVW_UABS_G2	\.data\+0x8
  28:	f2a00001 	movk	x1, #0x0, lsl #16
			28: R_AARCH64_MOVW_UABS_G1_NC	\.data\+0x8
  2c:	f2800001 	movk	x1, #0x0
			2c: R_AARCH64_MOVW_UABS_G0_NC	\.data\+0x8
  30:	d2c00001 	movz	x1, #0x0, lsl #32
			30: R_AARCH64_MOVW_UABS_G2	xdata
  34:	f2a00001 	movk	x1, #0x0, lsl #16
			34: R_AARCH64_MOVW_UABS_G1_NC	xdata
  38:	f2800001 	movk	x1, #0x0
			38: R_AARCH64_MOVW_UABS_G0_NC	xdata
  3c:	92c00001 	movn	x1, #0x0, lsl #32
  40:	f2bfffe1 	movk	x1, #0xffff, lsl #16
  44:	f29f0001 	movk	x1, #0xf800
  48:	d2ffffe1 	mov	x1, #0xffff000000000000    	// #-281474976710656
  4c:	f2dfffe1 	movk	x1, #0xffff, lsl #32
  50:	f2bfffe1 	movk	x1, #0xffff, lsl #16
  54:	f29f0001 	movk	x1, #0xf800
  58:	d2ffdb81 	mov	x1, #0xfedc000000000000    	// #-82190693199511552
  5c:	f2d75301 	movk	x1, #0xba98, lsl #32
  60:	f2aeca81 	movk	x1, #0x7654, lsl #16
  64:	f2864201 	movk	x1, #0x3210
  68:	580009a0 	ldr	x0, 19c <llit>
  6c:	58000001 	ldr	x1, 0 <func>
			6c: R_AARCH64_LD_PREL_LO19	\.data\+0x8
  70:	58000002 	ldr	x2, 0 <xdata>
			70: R_AARCH64_LD_PREL_LO19	xdata\+0xc
  74:	10000940 	adr	x0, 19c <llit>
  78:	10000001 	adr	x1, 0 <func>
			78: R_AARCH64_ADR_PREL_LO21	\.data\+0x8
  7c:	10000002 	adr	x2, 0 <func>
			7c: R_AARCH64_ADR_PREL_LO21	\.data\+0x1000
  80:	10000003 	adr	x3, 0 <xlit>
			80: R_AARCH64_ADR_PREL_LO21	xlit
  84:	10000004 	adr	x4, 0 <xdata>
			84: R_AARCH64_ADR_PREL_LO21	xdata\+0x10
  88:	10000005 	adr	x5, 0 <xdata>
			88: R_AARCH64_ADR_PREL_LO21	xdata\+0xff8
  8c:	90000000 	adrp	x0, 0 <func>
			8c: R_AARCH64_ADR_PREL_PG_HI21	\.text\+0x19c
  90:	90000001 	adrp	x1, 0 <func>
			90: R_AARCH64_ADR_PREL_PG_HI21	\.data\+0x8
  94:	90000002 	adrp	x2, 0 <func>
			94: R_AARCH64_ADR_PREL_PG_HI21	\.data\+0x1000
  98:	90000003 	adrp	x3, 0 <xlit>
			98: R_AARCH64_ADR_PREL_PG_HI21	xlit
  9c:	90000004 	adrp	x4, 0 <xdata>
			9c: R_AARCH64_ADR_PREL_PG_HI21	xdata\+0x10
  a0:	90000005 	adrp	x5, 0 <xdata>
			a0: R_AARCH64_ADR_PREL_PG_HI21	xdata\+0xff8
  a4:	90000000 	adrp	x0, 0 <func>
			a4: R_AARCH64_ADR_PREL_PG_HI21	\.text\+0x19c
  a8:	90000001 	adrp	x1, 0 <func>
			a8: R_AARCH64_ADR_PREL_PG_HI21	\.data\+0x8
  ac:	90000002 	adrp	x2, 0 <func>
			ac: R_AARCH64_ADR_PREL_PG_HI21	\.data\+0x1000
  b0:	90000003 	adrp	x3, 0 <xlit>
			b0: R_AARCH64_ADR_PREL_PG_HI21	xlit
  b4:	90000004 	adrp	x4, 0 <xdata>
			b4: R_AARCH64_ADR_PREL_PG_HI21	xdata\+0x10
  b8:	90000005 	adrp	x5, 0 <xdata>
			b8: R_AARCH64_ADR_PREL_PG_HI21	xdata\+0xff8
  bc:	91000000 	add	x0, x0, #0x0
			bc: R_AARCH64_ADD_ABS_LO12_NC	\.text\+0x19c
  c0:	91000021 	add	x1, x1, #0x0
			c0: R_AARCH64_ADD_ABS_LO12_NC	\.data\+0x8
  c4:	91000042 	add	x2, x2, #0x0
			c4: R_AARCH64_ADD_ABS_LO12_NC	\.data\+0x1000
  c8:	91000063 	add	x3, x3, #0x0
			c8: R_AARCH64_ADD_ABS_LO12_NC	xlit
  cc:	91000084 	add	x4, x4, #0x0
			cc: R_AARCH64_ADD_ABS_LO12_NC	xdata\+0x10
  d0:	910000a5 	add	x5, x5, #0x0
			d0: R_AARCH64_ADD_ABS_LO12_NC	xdata\+0xff8
  d4:	913ffcc6 	add	x6, x6, #0xfff
  d8:	39400000 	ldrb	w0, \[x0\]
			d8: R_AARCH64_LDST8_ABS_LO12_NC	\.text\+0x19c
  dc:	39400021 	ldrb	w1, \[x1\]
			dc: R_AARCH64_LDST8_ABS_LO12_NC	\.data\+0x8
  e0:	39400042 	ldrb	w2, \[x2\]
			e0: R_AARCH64_LDST8_ABS_LO12_NC	\.data\+0x1000
  e4:	39400063 	ldrb	w3, \[x3\]
			e4: R_AARCH64_LDST8_ABS_LO12_NC	xlit
  e8:	39400084 	ldrb	w4, \[x4\]
			e8: R_AARCH64_LDST8_ABS_LO12_NC	xdata\+0x10
  ec:	394000a5 	ldrb	w5, \[x5\]
			ec: R_AARCH64_LDST8_ABS_LO12_NC	xdata\+0xff8
  f0:	397ffcc6 	ldrb	w6, \[x6, #4095\]
  f4:	36000560 	tbz	w0, #0, 1a0 <lab>
  f8:	b6f80001 	tbz	x1, #63, 0 <xlab>
			f8: R_AARCH64_TSTBR14	xlab
  fc:	37400522 	tbnz	w2, #8, 1a0 <lab>
 100:	b7780002 	tbnz	x2, #47, 0 <xlab>
			100: R_AARCH64_TSTBR14	xlab
 104:	540004e0 	b\.eq	1a0 <lab>  // b\.none
 108:	54000000 	b\.eq	0 <xlab>  // b\.none
			108: R_AARCH64_CONDBR19	xlab
 10c:	b40004a0 	cbz	x0, 1a0 <lab>
 110:	b500001e 	cbnz	x30, 0 <xlab>
			110: R_AARCH64_CONDBR19	xlab
 114:	14000023 	b	1a0 <lab>
 118:	14000000 	b	0 <xlab>
			118: R_AARCH64_JUMP26	xlab
 11c:	94000021 	bl	1a0 <lab>
 120:	94000000 	bl	0 <xlab>
			120: R_AARCH64_CALL26	xlab
 124:	d2e24680 	mov	x0, #0x1234000000000000    	// #1311673391471656960
 128:	f2cacf00 	movk	x0, #0x5678, lsl #32
 12c:	f2b35780 	movk	x0, #0x9abc, lsl #16
 130:	f29bde00 	movk	x0, #0xdef0
 134:	d2ffdb80 	mov	x0, #0xfedc000000000000    	// #-82190693199511552
 138:	f2d75300 	movk	x0, #0xba98, lsl #32
 13c:	f2aeca80 	movk	x0, #0x7654, lsl #16
 140:	f2864200 	movk	x0, #0x3210
 144:	b2440c00 	orr	x0, x0, #0xf000000000000000
 148:	927cec00 	and	x0, x0, #0xfffffffffffffff0
 14c:	121c6c00 	and	w0, w0, #0xfffffff0
 150:	d1200000 	sub	x0, x0, #0x800
 154:	913ffc00 	add	x0, x0, #0xfff
 158:	91200000 	add	x0, x0, #0x800
 15c:	d13ffc00 	sub	x0, x0, #0xfff
 160:	d41fffe1 	svc	#0xffff
 164:	f8500420 	ldr	x0, \[x1\], #-256
 168:	f8500c20 	ldr	x0, \[x1, #-256\]!
 16c:	f8500020 	ldur	x0, \[x1, #-256\]
 170:	f97ffc20 	ldr	x0, \[x1, #32760\]
 174:	79400000 	ldrh	w0, \[x0\]
			174: R_AARCH64_LDST16_ABS_LO12_NC	\.text\+0x19c
 178:	b9400021 	ldr	w1, \[x1\]
			178: R_AARCH64_LDST32_ABS_LO12_NC	\.data\+0x8
 17c:	f9400042 	ldr	x2, \[x2\]
			17c: R_AARCH64_LDST64_ABS_LO12_NC	\.data\+0x1000
 180:	3dc00063 	ldr	q3, \[x3\]
			180: R_AARCH64_LDST128_ABS_LO12_NC	xlit
 184:	f98000f0 	prfm	pstl1keep, \[x7\]
			184: R_AARCH64_LDST64_ABS_LO12_NC	\.data\+0x100c
 188:	58000000 	ldr	x0, 1 <func\+0x1>
			188: R_AARCH64_GOT_LD_PREL19	cdata
 18c:	39400001 	ldrb	w1, \[x0\]
 190:	d65f03c0 	ret
 194:	f94001bc 	ldr	x28, \[x13\]
			194: R_AARCH64_LD64_GOTPAGE_LO15	\.data
 198:	f9400000 	ldr	x0, \[x0\]
			198: R_AARCH64_LD64_GOTOFF_LO15	.data

000000000000019c <llit>:
 19c:	deadf00d 	\.word	0xdeadf00d
