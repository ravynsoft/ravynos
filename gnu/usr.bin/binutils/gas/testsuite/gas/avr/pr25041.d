#name: PR 25041 (correct generation of lds/sts addresses)
#as: -m "attiny10"
#target: avr-*-*
#objdump: -Dm"avr:100"

#...
00000000 <_start>:
   0:	00 a0       	lds	r16, 0x80	; 0x800080 <_start\+0x800080>
   2:	00 a8       	sts	0x80, r16	; 0x800080 <_start\+0x800080>
