#name: AVR (avr51) check disassembly if symbolic name present
#as: -mavr51
#ld: -mavr51
#source: lds-mega.s
#objdump: -d
#target: avr-*-*

.*:     file format elf32-avr


Disassembly of section .text:

00000000 <main>:
   0:	80 91 00 01 	lds	r24, 0x0100	; 0x800100 <myvar1>
   4:	08 2e       	mov	r0, r24
   6:	00 0c       	add	r0, r0
   8:	99 0b       	sbc	r25, r25
   a:	90 93 03 01 	sts	0x0103, r25	; 0x800103 <myvar2\+0x1>
   e:	80 93 02 01 	sts	0x0102, r24	; 0x800102 <myvar2>
  12:	80 e0       	ldi	r24, 0x00	; 0
  14:	90 e0       	ldi	r25, 0x00	; 0
  16:	08 95       	ret

