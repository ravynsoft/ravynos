#name: aarch64-farcall-bl-section
#source: farcall-bl-section.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x8001000
#objdump: -dr
#...

Disassembly of section .text:

.* <_start>:
    1000:	9400000a 	bl	1028 <___veneer>
    1004:	94000005 	bl	1018 <___veneer>
    1008:	d65f03c0 	ret
    100c:	d503201f 	nop
    1010:	1400000e 	b	1048 <___veneer\+0x20>
    1014:	d503201f 	nop

.* <___veneer>:
    1018:	90040010 	adrp	x16, 8001000 <bar>
    101c:	91001210 	add	x16, x16, #0x4
    1020:	d61f0200 	br	x16
    1024:	00000000 	udf	#0

.* <___veneer>:
    1028:	90040010 	adrp	x16, 8001000 <bar>
    102c:	91000210 	add	x16, x16, #0x0
    1030:	d61f0200 	br	x16
	...

Disassembly of section .foo:

.* <bar>:
 8001000:	d65f03c0 	ret

.* <bar2>:
 8001004:	d65f03c0 	ret
