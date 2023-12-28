#name: AVR .avr.prop, AVR_7_PCREL after align
#as: -mavrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-7.s
#objdump: -S
#target: avr-*-*

#...
00000000 <__ctors_end>:
   0:	04 d0       	rcall	.+8      	; 0xa <foo>
   2:	00 00       	nop
#...
   6:	86 e0       	ldi	r24, 0x06	; 6
   8:	f0 f7       	brcc	.-4      	; 0x6 <.*>
#...
