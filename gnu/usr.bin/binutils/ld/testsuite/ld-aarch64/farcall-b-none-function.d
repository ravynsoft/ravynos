#name: aarch64-farcall-b-none-function
#source: farcall-b-none-function.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x8001000
#objdump: -dr
#...

Disassembly of section .text:

.* <_start>:
    1000:	14000004 	b	1010 <__bar_veneer>
    1004:	d65f03c0 	ret
    1008:	14000008 	b	1028 <__bar_veneer\+0x18>
    100c:	d503201f 	nop

.* <__bar_veneer>:
    1010:	90040010 	adrp	x16, 8001000 <bar>
    1014:	91000210 	add	x16, x16, #0x0
    1018:	d61f0200 	br	x16
	...

Disassembly of section .foo:

.* <bar>:
 8001000:	d65f03c0 	ret
