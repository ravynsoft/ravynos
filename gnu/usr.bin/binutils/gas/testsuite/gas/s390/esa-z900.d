#name: s390 opcode
#objdump: -drw

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	c0 f4 00 00 00 00 [	 ]*jg	0 <foo>
.*:	c0 04 00 00 00 00 [	 ]*jgnop	6 <foo\+0x6>
.*:	c0 14 00 00 00 00 [	 ]*jgo	c <foo\+0xc>
.*:	c0 24 00 00 00 00 [	 ]*jgh	12 <foo\+0x12>
.*:	c0 24 00 00 00 00 [	 ]*jgh	18 <foo\+0x18>
.*:	c0 34 00 00 00 00 [	 ]*jgnle	1e <foo\+0x1e>
.*:	c0 44 00 00 00 00 [	 ]*jgl	24 <foo\+0x24>
.*:	c0 44 00 00 00 00 [	 ]*jgl	2a <foo\+0x2a>
.*:	c0 54 00 00 00 00 [	 ]*jgnhe	30 <foo\+0x30>
.*:	c0 64 00 00 00 00 [	 ]*jglh	36 <foo\+0x36>
.*:	c0 74 00 00 00 00 [	 ]*jgne	3c <foo\+0x3c>
.*:	c0 74 00 00 00 00 [	 ]*jgne	42 <foo\+0x42>
.*:	c0 84 00 00 00 00 [	 ]*jge	48 <foo\+0x48>
.*:	c0 84 00 00 00 00 [	 ]*jge	4e <foo\+0x4e>
.*:	c0 94 00 00 00 00 [	 ]*jgnlh	54 <foo\+0x54>
.*:	c0 a4 00 00 00 00 [	 ]*jghe	5a <foo\+0x5a>
.*:	c0 b4 00 00 00 00 [	 ]*jgnl	60 <foo\+0x60>
.*:	c0 b4 00 00 00 00 [	 ]*jgnl	66 <foo\+0x66>
.*:	c0 c4 00 00 00 00 [	 ]*jgle	6c <foo\+0x6c>
.*:	c0 d4 00 00 00 00 [	 ]*jgnh	72 <foo\+0x72>
.*:	c0 d4 00 00 00 00 [	 ]*jgnh	78 <foo\+0x78>
.*:	c0 e4 00 00 00 00 [	 ]*jgno	7e <foo\+0x7e>
.*:	c0 f4 00 00 00 00 [	 ]*jg	84 <foo\+0x84>
.*:	c0 14 00 00 00 00 [	 ]*jgo	8a <foo\+0x8a>
.*:	c0 24 00 00 00 00 [	 ]*jgh	90 <foo\+0x90>
.*:	c0 24 00 00 00 00 [	 ]*jgh	96 <foo\+0x96>
.*:	c0 34 00 00 00 00 [	 ]*jgnle	9c <foo\+0x9c>
.*:	c0 44 00 00 00 00 [	 ]*jgl	a2 <foo\+0xa2>
.*:	c0 44 00 00 00 00 [	 ]*jgl	a8 <foo\+0xa8>
.*:	c0 54 00 00 00 00 [	 ]*jgnhe	ae <foo\+0xae>
.*:	c0 64 00 00 00 00 [	 ]*jglh	b4 <foo\+0xb4>
.*:	c0 74 00 00 00 00 [	 ]*jgne	ba <foo\+0xba>
.*:	c0 74 00 00 00 00 [	 ]*jgne	c0 <foo\+0xc0>
.*:	c0 84 00 00 00 00 [	 ]*jge	c6 <foo\+0xc6>
.*:	c0 84 00 00 00 00 [	 ]*jge	cc <foo\+0xcc>
.*:	c0 94 00 00 00 00 [	 ]*jgnlh	d2 <foo\+0xd2>
.*:	c0 a4 00 00 00 00 [	 ]*jghe	d8 <foo\+0xd8>
.*:	c0 b4 00 00 00 00 [	 ]*jgnl	de <foo\+0xde>
.*:	c0 b4 00 00 00 00 [	 ]*jgnl	e4 <foo\+0xe4>
.*:	c0 c4 00 00 00 00 [	 ]*jgle	ea <foo\+0xea>
.*:	c0 d4 00 00 00 00 [	 ]*jgnh	f0 <foo\+0xf0>
.*:	c0 d4 00 00 00 00 [	 ]*jgnh	f6 <foo\+0xf6>
.*:	c0 e4 00 00 00 00 [	 ]*jgno	fc <foo\+0xfc>
.*:	c0 f4 00 00 00 00 [	 ]*jg	102 <foo\+0x102>
.*:	c0 65 00 00 00 00 [	 ]*brasl	%r6,108 <foo\+0x108>
.*:	c0 65 00 00 00 00 [	 ]*brasl	%r6,10e <foo\+0x10e>
.*:	c0 65 80 00 00 00 [	 ]*brasl	%r6,114 <foo\+0x114>
.*:	c0 65 80 00 00 00 [	 ]*brasl	%r6,11a <foo\+0x11a>
.*:	c0 65 7f ff ff ff [	 ]*brasl	%r6,11e <foo\+0x11e>
.*:	c0 65 7f ff ff ff [	 ]*brasl	%r6,124 <foo\+0x124>
.*:	01 0b [	 ]*tam
.*:	01 0c [	 ]*sam24
.*:	01 0d [	 ]*sam31
.*:	b2 b1 5f ff [	 ]*stfl	4095\(%r5\)
.*:	b9 1f 00 69 [	 ]*lrvr	%r6,%r9
.*:	b9 8d 00 69 [	 ]*epsw	%r6,%r9
.*:	b9 96 00 69 [	 ]*mlr	%r6,%r9
.*:	b9 97 00 69 [	 ]*dlr	%r6,%r9
.*:	b9 98 00 69 [	 ]*alcr	%r6,%r9
.*:	b9 99 00 69 [	 ]*slbr	%r6,%r9
.*:	c0 60 00 00 00 00 [	 ]*larl	%r6,14e <foo\+0x14e>
.*:	e3 65 af ff 00 1e [	 ]*lrv	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 1f [	 ]*lrvh	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 3e [	 ]*strv	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 3f [	 ]*strvh	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 96 [	 ]*ml	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 97 [	 ]*dl	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 98 [	 ]*alc	%r6,4095\(%r5,%r10\)
.*:	e3 65 af ff 00 99 [	 ]*slb	%r6,4095\(%r5,%r10\)
.*:	eb 69 5f ff 00 1d [	 ]*rll	%r6,%r9,4095\(%r5\)
.*:	07 07 [	 ]*nopr	%r7
