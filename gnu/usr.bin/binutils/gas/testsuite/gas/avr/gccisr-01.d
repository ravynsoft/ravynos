#name: gccisr-01: __gcc_isr pseudo instruction
#as: -mgcc-isr -mavr4
#objdump: -dz
#target: avr-*-*

.*: +file format elf32-avr


Disassembly of section \.text:

00000000 <__start1>:
   0:	68 94       	set

00000002 <__vec1_start>:
   2:	0f 92       	push	r0
   4:	0f b6       	in	r0, 0x3f	; 63
   6:	0f 92       	push	r0
   8:	01 30       	cpi	r16, 0x01	; 1
   a:	0f 90       	pop	r0
   c:	0f be       	out	0x3f, r0	; 63
   e:	0f 90       	pop	r0
  10:	e8 94       	clt

00000012 <__data1>:
  12:	00 e0       	ldi	r16, 0x00	; 0
  14:	08 00       	\.word	0x0008	; \?\?\?\?

00000016 <__start2>:
  16:	68 94       	set

00000018 <__vec2_start>:
  18:	e1 e0       	ldi	r30, 0x01	; 1
  1a:	f0 91 00 00 	lds	r31, 0x0000	; 0x800000 <__data6\+0x7fff40>
  1e:	f0 93 00 00 	sts	0x0000, r31	; 0x800000 <__data6\+0x7fff40>
  22:	12 01       	movw	r2, r4
  24:	12 95       	swap	r17
  26:	18 95       	reti
  28:	78 10       	cpse	r7, r8
  2a:	78 94       	sei
  2c:	f8 94       	cli
  2e:	af b6       	in	r10, 0x3f	; 63
  30:	af be       	out	0x3f, r10	; 63
  32:	18 95       	reti
  34:	e8 94       	clt

00000036 <__data2>:
  36:	00 e0       	ldi	r16, 0x00	; 0
  38:	0f 00       	\.word	0x000f	; \?\?\?\?

0000003a <__start3>:
  3a:	68 94       	set

0000003c <__vec3_start>:
  3c:	1f 92       	push	r1
  3e:	1f b6       	in	r1, 0x3f	; 63
  40:	1f 92       	push	r1
  42:	11 24       	eor	r1, r1
  44:	8f 93       	push	r24
  46:	8f 91       	pop	r24
  48:	1f 90       	pop	r1
  4a:	1f be       	out	0x3f, r1	; 63
  4c:	1f 90       	pop	r1
  4e:	18 95       	reti
  50:	8f 91       	pop	r24
  52:	1f 90       	pop	r1
  54:	1f be       	out	0x3f, r1	; 63
  56:	1f 90       	pop	r1
  58:	18 95       	reti
  5a:	13 94       	inc	r1
  5c:	e8 94       	clt

0000005e <__data3>:
  5e:	00 e0       	ldi	r16, 0x00	; 0
  60:	11 00       	\.word	0x0011	; \?\?\?\?

00000062 <__start4>:
  62:	68 94       	set

00000064 <__vec4_start>:
  64:	0f 92       	push	r0
  66:	0f b6       	in	r0, 0x3f	; 63
  68:	0f 92       	push	r0
  6a:	1f 92       	push	r1
  6c:	11 24       	eor	r1, r1
  6e:	8f 93       	push	r24
  70:	8f 91       	pop	r24
  72:	1f 90       	pop	r1
  74:	0f 90       	pop	r0
  76:	0f be       	out	0x3f, r0	; 63
  78:	0f 90       	pop	r0
  7a:	18 95       	reti
  7c:	8f 91       	pop	r24
  7e:	1f 90       	pop	r1
  80:	0f 90       	pop	r0
  82:	0f be       	out	0x3f, r0	; 63
  84:	0f 90       	pop	r0
  86:	18 95       	reti
  88:	01 9f       	mul	r16, r17
  8a:	e8 94       	clt

0000008c <__data4>:
  8c:	00 e0       	ldi	r16, 0x00	; 0
  8e:	14 00       	\.word	0x0014	; \?\?\?\?

00000090 <__start5>:
  90:	68 94       	set

00000092 <__vec5_start>:
  92:	0f 92       	push	r0
  94:	c8 95       	lpm
  96:	0f 90       	pop	r0
  98:	18 95       	reti
  9a:	0f 90       	pop	r0
  9c:	18 95       	reti
  9e:	e8 94       	clt

000000a0 <__data5>:
  a0:	00 e0       	ldi	r16, 0x00	; 0
  a2:	07 00       	\.word	0x0007	; \?\?\?\?

000000a4 <__start6>:
  a4:	68 94       	set

000000a6 <__vec6_start>:
  a6:	af 93       	push	r26
  a8:	af b7       	in	r26, 0x3f	; 63
  aa:	af 93       	push	r26
  ac:	af 91       	pop	r26
  ae:	af bf       	out	0x3f, r26	; 63
  b0:	af 91       	pop	r26
  b2:	18 95       	reti
  b4:	af 91       	pop	r26
  b6:	af bf       	out	0x3f, r26	; 63
  b8:	af 91       	pop	r26
  ba:	18 95       	reti
  bc:	88 94       	clc
  be:	e8 94       	clt

000000c0 <__data6>:
  c0:	00 e0       	ldi	r16, 0x00	; 0
  c2:	0d 00       	\.word	0x000d	; \?\?\?\?
