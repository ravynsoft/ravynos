#name: AVR .avr.prop, AVR_7_PCREL just before align
#as: -mavrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-8.s
#objdump: -S
#target: avr-*-*

#...
00000000 <__ctors_end>:
   0:	ff cf       	rjmp	.-2      	; 0x0 <__ctors_end>
   2:	fe df       	rcall	.-4      	; 0x0 <__ctors_end>
#...
   4:	f8 f7       	brcc	.-2      	; 0x4 <.*>
#...
