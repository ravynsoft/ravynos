#name: aarch64-farcall-b-defsym
#source: farcall-b-defsym.s
#as:
#ld: -Ttext 0x1000 --defsym=bar=0x8001000
#objdump: -dr
#...

Disassembly of section .text:

0+1000 <_start>:
 +1000:	14000004 	b	1010 <__bar_veneer>
 +1004:	d65f03c0 	ret
[ \t]+1008:[ \t]+14000008[ \t]+b[ \t]+1028 <__bar_veneer\+0x18>
[ \t]+100c:[ \t]+d503201f[ \t]+nop
0+1010 <__bar_veneer>:
    1010:	90040010 	adrp	x16, 8001000 <bar>
    1014:	91000210 	add	x16, x16, #0x0
    1018:	d61f0200 	br	x16
	...
