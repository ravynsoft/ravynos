#name: Local BLX instructions in Thumb mode.
#objdump: -drw --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince *-*nto* *-*netbsd*
#as:
#warning_output: blx-local-thumb.l

.*: +file format .*arm.*
Disassembly of section .text:
[^<]*<one> f000 f80e 	bl	00000020 <foo>
[^<]*<one\+0x4> f000 e812 	blx	0000002c <foo2>
[^<]*<one\+0x8> f000 f80a 	bl	00000020 <foo>
[^<]*<one\+0xc> f000 e80e 	blx	0000002c <foo2>
[^<]*<one\+0x10> f000 e80e 	blx	00000030 <fooundefarm>
[^<]*<one\+0x14> f000 f80c 	bl	00000030 <fooundefarm>
[^<]*<one\+0x18> f000 e806 	blx	00000028 <fooundefthumb>
[^<]*<one\+0x1c> f000 f804 	bl	00000028 <fooundefthumb>
[^<]*<foo> e7ee      	b.n	00000000 <one>
[^<]*<foo\+0x2> e003      	b.n	0000002c <foo2>
[^<]*<foo\+0x4> 46c0      	nop			@ \(mov r8, r8\)
[^<]*<foo\+0x6> 46c0      	nop			@ \(mov r8, r8\)
[^<]*<fooundefthumb> 46c0      	nop			@ \(mov r8, r8\)
	...
[^<]*<foo2> e1a00000 	nop			@ \(mov r0, r0\)
[^<]*<fooundefarm> e1a00000 	nop			@ \(mov r0, r0\)
