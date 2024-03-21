#name: aarch64-farcall-back
#source: farcall-back.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x100000000
#notarget: aarch64_be-*-*
#objdump: -dr
#...

Disassembly of section .text:

0000000000001000 <_start>:
    1000:	14000414 	b	2050 <__bar1_veneer>
    1004:	94000413 	bl	2050 <__bar1_veneer>
    1008:	14000408 	b	2028 <__bar2_veneer>
    100c:	94000407 	bl	2028 <__bar2_veneer>
    1010:	1400040a 	b	2038 <__bar3_veneer>
    1014:	94000409 	bl	2038 <__bar3_veneer>
    1018:	d65f03c0 	ret
	...

000000000000201c <_back>:
    201c:	d65f03c0 	ret

[ \t]+2020:[ \t]+14000014[ \t]+b[ \t]+2070 <__bar1_veneer\+0x20>
[ \t]+2024:[ \t]+d503201f[ \t]+nop
0000000000002028 <__bar2_veneer>:
    2028:	f07ffff0 	adrp	x16, 100001000 <bar1\+0x1000>
    202c:	91002210 	add	x16, x16, #0x8
    2030:	d61f0200 	br	x16
    2034:	00000000 	udf	#0

0000000000002038 <__bar3_veneer>:
    2038:	58000090 	ldr	x16, 2048 <__bar3_veneer\+0x10>
    203c:	10000011 	adr	x17, 203c <__bar3_veneer\+0x4>
    2040:	8b110210 	add	x16, x16, x17
    2044:	d61f0200 	br	x16
    2048:	ffffffd4 	.word	0xffffffd4
    204c:	00000000 	.word	0x00000000

0000000000002050 <__bar1_veneer>:
    2050:	d07ffff0 	adrp	x16, 100000000 <bar1>
    2054:	91000210 	add	x16, x16, #0x0
    2058:	d61f0200 	br	x16
	...

Disassembly of section .foo:

0000000100000000 <bar1>:
   100000000:	d65f03c0 	ret
   100000004:	14000807 	b	100002020 <___start_veneer>
	...

0000000100001008 <bar2>:
   100001008:	d65f03c0 	ret
   10000100c:	14000405 	b	100002020 <___start_veneer>
	...

0000000100002010 <bar3>:
   100002010:	d65f03c0 	ret
   100002014:	14000009 	b	100002038 <___back_veneer>

[ \t]+100002018:[ \t]+1400000e[ \t]+b[ \t]+100002050 <___back_veneer\+0x18>
[ \t]+10000201c:[ \t]+d503201f[ \t]+nop
0000000100002020 <___start_veneer>:
   100002020:	58000090 	ldr	x16, 100002030 <___start_veneer\+0x10>
   100002024:	10000011 	adr	x17, 100002024 <___start_veneer\+0x4>
   100002028:	8b110210 	add	x16, x16, x17
   10000202c:	d61f0200 	br	x16
   100002030:	ffffefdc 	.word	0xffffefdc
   100002034:	fffffffe 	.word	0xfffffffe

0000000100002038 <___back_veneer>:
   100002038:	90800010 	adrp	x16, 2000 <_start\+0x1000>
   10000203c:	91007210 	add	x16, x16, #0x1c
   100002040:	d61f0200 	br	x16
	...
