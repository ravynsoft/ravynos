#name: AVR RMW instructions
#as: -mmcu=avrxmega2 -mrmw
#objdump: -dr --show-raw-insn
#target: avr-*-*

.*: +file format elf32-avr

Disassembly of section .text:

00000000 <main>:
   0:	cf 93       	push	r28
   2:	df 93       	push	r29
   4:	cd b7       	in	r28, 0x3d	; 61
   6:	de b7       	in	r29, 0x3e	; 62
   8:	c4 92       	xch	Z, r12
   a:	c5 92       	las	Z, r12
   c:	c6 92       	lac	Z, r12
   e:	c7 92       	lat	Z, r12
  10:	80 e0       	ldi	r24, 0x00	; 0
  12:	90 e0       	ldi	r25, 0x00	; 0
  14:	df 91       	pop	r29
  16:	cf 91       	pop	r28
  18:	08 95       	ret
