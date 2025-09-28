#name: gccisr-02: __gcc_isr pseudo instruction
#as: -mgcc-isr -mavrtiny
#objdump: -dz
#target: avr-*-*

.*: +file format elf32-avr


Disassembly of section \.text:

00000000 <__start1>:
   0:	68 94       	set

00000002 <__vec1_start>:
   2:	0f 93       	push	r16
   4:	0f b7       	in	r16, 0x3f	; 63
   6:	0f 93       	push	r16
   8:	21 30       	cpi	r18, 0x01	; 1
   a:	0f 91       	pop	r16
   c:	0f bf       	out	0x3f, r16	; 63
   e:	0f 91       	pop	r16
  10:	e8 94       	clt

00000012 <__data1>:
  12:	00 e0       	ldi	r16, 0x00	; 0
  14:	08 00       	\.word	0x0008	; \?\?\?\?

00000016 <__start2>:
  16:	68 94       	set

00000018 <__vec2_start>:
  18:	1f 93       	push	r17
  1a:	10 e0       	ldi	r17, 0x00	; 0
  1c:	1f 91       	pop	r17
  1e:	18 95       	reti
  20:	e1 2f       	mov	r30, r17
  22:	1f 91       	pop	r17
  24:	18 95       	reti
  26:	e8 94       	clt

00000028 <__data2>:
  28:	00 e0       	ldi	r16, 0x00	; 0
  2a:	08 00       	\.word	0x0008	; \?\?\?\?
