#name: AVR .avr.prop, single .align sym at end of section test.
#as: -mavrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-6.s
#objdump: -S
#target: avr-*-*

#...
   0:	00 c0       	rjmp	.+0      	; 0x2 <dest>

00000002 <dest>:
   2:	00 00       	nop
   4:	fe cf       	rjmp	.-4      	; 0x2 <dest>
#...
