
.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <_start>:
 +[0-9a-f]+:	[0-9a-f]{8} 	bl	[0-9a-f]+ <__bar_from_arm>
 +[0-9a-f]+:	[0-9a-f]{8} 	bl	[0-9a-f]+ <__bar2_veneer>
 +[0-9a-f]+:	[0-9a-f]{8} 	bl	[0-9a-f]+ <__bar3_veneer>
 +[0-9a-f]+:	[0-9a-f]{8} 	bl	[0-9a-f]+ <__bar4_from_arm>
 +[0-9a-f]+:	[0-9a-f]{8} 	bl	[0-9a-f]+ <__bar5_from_arm>
 +[0-9a-f]+:	00000000 	andeq	r0, r0, r0

[0-9a-f]+ <__bar_from_arm>:
 +[0-9a-f]+:	e59fc000 	ldr	ip, \[pc\]	@ [0-9a-f]+ <__bar_from_arm\+0x8>
 +[0-9a-f]+:	e12fff1c 	bx	ip
 +[0-9a-f]+:	02002021 	.word	0x02002021
[0-9a-f]+ <__bar3_veneer>:
 +[0-9a-f]+:	e51ff004 	ldr	pc, \[pc, #-4\]	@ [0-9a-f]+ <__bar3_veneer\+0x4>
 +[0-9a-f]+:	02002028 	.word	0x02002028
[0-9a-f]+ <__bar5_from_arm>:
 +[0-9a-f]+:	e59fc000 	ldr	ip, \[pc\]	@ [0-9a-f]+ <__bar5_from_arm\+0x8>
 +[0-9a-f]+:	e12fff1c 	bx	ip
 +[0-9a-f]+:	0200202f 	.word	0x0200202f
[0-9a-f]+ <__bar4_from_arm>:
 +[0-9a-f]+:	e59fc000 	ldr	ip, \[pc\]	@ [0-9a-f]+ <__bar4_from_arm\+0x8>
 +[0-9a-f]+:	e12fff1c 	bx	ip
 +[0-9a-f]+:	0200202d 	.word	0x0200202d

[0-9a-f]+ <__bar2_veneer>:
 +[0-9a-f]+:	e51ff004 	ldr	pc, \[pc, #-4\]	@ [0-9a-f]+ <__bar2_veneer\+0x4>
 +[0-9a-f]+:	02002024 	.word	0x02002024
	...

Disassembly of section .foo:

[0-9a-f]+ <bar>:
 +[0-9a-f]+:	4770      	bx	lr
	...

[0-9a-f]+ <bar2>:
 +[0-9a-f]+:	e12fff1e 	bx	lr

[0-9a-f]+ <bar3>:
 +[0-9a-f]+:	e12fff1e 	bx	lr

[0-9a-f]+ <bar4>:
 +[0-9a-f]+:	4770      	bx	lr

[0-9a-f]+ <bar5>:
 +[0-9a-f]+:	4770      	bx	lr
