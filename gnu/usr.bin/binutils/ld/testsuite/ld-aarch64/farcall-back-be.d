#name: aarch64-farcall-back-be
#source: farcall-back.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x100000000
#objdump: -dr
#target: aarch64_be-*-*
#...

Disassembly of section .text:

0000000000001000 <_start>:
    1000:	14000413 	b	204c <__bar1_veneer>
    1004:	94000412 	bl	204c <__bar1_veneer>
    1008:	14000407 	b	2024 <__bar2_veneer>
    100c:	94000406 	bl	2024 <__bar2_veneer>
    1010:	14000409 	b	2034 <__bar3_veneer>
    1014:	94000408 	bl	2034 <__bar3_veneer>
    1018:	d65f03c0 	ret
	...

000000000000201c <_back>:
    201c:	d65f03c0 	ret

[ \t]+2020:[ \t]+14000013[ \t]+b[ \t]+206c <__bar1_veneer\+0x20>
0000000000002024 <__bar2_veneer>:
    2024:	f07ffff0 	adrp	x16, 100001000 <bar1\+0x1000>
    2028:	91002210 	add	x16, x16, #0x8
    202c:	d61f0200 	br	x16
    2030:	00000000 	.inst	0x00000000 ; undefined

0000000000002034 <__bar3_veneer>:
    2034:	58000090 	ldr	x16, 2044 <__bar3_veneer\+0x10>
    2038:	10000011 	adr	x17, 2038 <__bar3_veneer\+0x4>
    203c:	8b110210 	add	x16, x16, x17
    2040:	d61f0200 	br	x16
    2044:	00000000 	.word	0x00000000
    2048:	ffffffd8 	.word	0xffffffd8

000000000000204c <__bar1_veneer>:
    204c:	d07ffff0 	adrp	x16, 100000000 <bar1>
    2050:	91000210 	add	x16, x16, #0x0
    2054:	d61f0200 	br	x16
	...

Disassembly of section .foo:

0000000100000000 <bar1>:
   100000000:	d65f03c0 	ret
   100000004:	14000806 	b	10000201c <___start_veneer>
	...

0000000100001008 <bar2>:
   100001008:	d65f03c0 	ret
   10000100c:	14000404 	b	10000201c <___start_veneer>
	...

0000000100002010 <bar3>:
   100002010:	d65f03c0 	ret
   100002014:	14000008 	b	100002034 <___back_veneer>

[ \t]+100002018:[ \t]+1400000d[ \t]+b[ \t]+10000204c <___back_veneer\+0x18>
000000010000201c <___start_veneer>:
   10000201c:	58000090 	ldr	x16, 10000202c <___start_veneer\+0x10>
   100002020:	10000011 	adr	x17, 100002020 <___start_veneer\+0x4>
   100002024:	8b110210 	add	x16, x16, x17
   100002028:	d61f0200 	br	x16
   10000202c:	fffffffe 	.word	0xfffffffe
   100002030:	ffffefe0 	.word	0xffffefe0

0000000100002034 <___back_veneer>:
   100002034:	90800010 	adrp	x16, 2000 <_start\+0x1000>
   100002038:	91007210 	add	x16, x16, #0x1c
   10000203c:	d61f0200 	br	x16
	...
