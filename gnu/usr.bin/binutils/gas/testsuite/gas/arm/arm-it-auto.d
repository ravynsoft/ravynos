#name: ARM IT automatic instruction generation
#as: -mthumb -march=armv7 -mimplicit-it=always
#objdump: -d --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <main> f000 f821 	bl	00000046 <main\+0x46>
00000004 <main\+0x4> f000 f80c 	bl	00000020 <main\+0x20>
00000008 <main\+0x8> f000 f813 	bl	00000032 <main\+0x32>
0000000c <main\+0xc> d142      	bne.n	00000094 <main\+0x94>
0000000e <main\+0xe> bf18      	it	ne
00000010 <main\+0x10> 4487      	addne	pc, r0
00000012 <main\+0x12> bf18      	it	ne
00000014 <main\+0x14> e8d0 f001 	tbbne	\[r0, r1\]
00000018 <main\+0x18> bf08      	it	eq
0000001a <main\+0x1a> e8d1 f010 	tbheq	\[r1, r0, lsl #1\]
0000001e <main\+0x1e> bf0a      	itet	eq
00000020 <main\+0x20> 2002      	moveq	r0, #2
00000022 <main\+0x22> 2003      	movne	r0, #3
00000024 <main\+0x24> 2004      	moveq	r0, #4
00000026 <main\+0x26> bf16      	itet	ne
00000028 <main\+0x28> 2002      	movne	r0, #2
0000002a <main\+0x2a> 2003      	moveq	r0, #3
0000002c <main\+0x2c> 2004      	movne	r0, #4
0000002e <main\+0x2e> bf18      	it	ne
00000030 <main\+0x30> 2001      	movne	r0, #1
00000032 <main\+0x32> bf0c      	ite	eq
00000034 <main\+0x34> 2002      	moveq	r0, #2
00000036 <main\+0x36> f8d1 f000 	ldrne.w	pc, \[r1\]
0000003a <main\+0x3a> bf18      	it	ne
0000003c <main\+0x3c> f000 f82a 	blne	00000094 <main\+0x94>
00000040 <main\+0x40> bfb8      	it	lt
00000042 <main\+0x42> f000 f828 	bllt	00000096 <main\+0x96>
00000046 <main\+0x46> bf17      	itett	ne
00000048 <main\+0x48> 202d      	movne	r0, #45.*
0000004a <main\+0x4a> 2005      	moveq	r0, #5
0000004c <main\+0x4c> 2006      	movne	r0, #6
0000004e <main\+0x4e> 4487      	addne	pc, r0
00000050 <main\+0x50> bf0d      	iteet	eq
00000052 <main\+0x52> 2007      	moveq	r0, #7
00000054 <main\+0x54> 2008      	movne	r0, #8
00000056 <main\+0x56> 2003      	movne	r0, #3
00000058 <main\+0x58> 2004      	moveq	r0, #4
0000005a <main\+0x5a> bf0b      	itete	eq
0000005c <main\+0x5c> 2005      	moveq	r0, #5
0000005e <main\+0x5e> 2006      	movne	r0, #6
00000060 <main\+0x60> 2007      	moveq	r0, #7
00000062 <main\+0x62> 2008      	movne	r0, #8
00000064 <main\+0x64> bf0c      	ite	eq
00000066 <main\+0x66> 2005      	moveq	r0, #5
00000068 <main\+0x68> 2006      	movne	r0, #6
0000006a <main\+0x6a> 4687      	mov	pc, r0
0000006c <main\+0x6c> bf0b      	itete	eq
0000006e <main\+0x6e> 2007      	moveq	r0, #7
00000070 <main\+0x70> 2008      	movne	r0, #8
00000072 <main\+0x72> 2005      	moveq	r0, #5
00000074 <main\+0x74> 2006      	movne	r0, #6
00000076 <main\+0x76> 4487      	add	pc, r0
00000078 <main\+0x78> bf0c      	ite	eq
0000007a <main\+0x7a> 2007      	moveq	r0, #7
0000007c <main\+0x7c> 2008      	movne	r0, #8
0000007e <main\+0x7e> bfcc      	ite	gt
00000080 <main\+0x80> 2009      	movgt	r0, #9
00000082 <main\+0x82> 200a      	movle	r0, #10
00000084 <main\+0x84> bf08      	it	eq
00000086 <main\+0x86> 200b      	moveq	r0, #11
00000088 <main\+0x88> bfd8      	it	le
0000008a <main\+0x8a> 200c      	movle	r0, #12
0000008c <main\+0x8c> bf18      	it	ne
0000008e <main\+0x8e> 200d      	movne	r0, #13
00000090 <main\+0x90> f... f... 	bl	0000000. <f.*>
00000094 <main\+0x94> bd10      	pop	{r4, pc}
00000096 <main\+0x96> f... f... 	bl	0000000. <f.*>
0000009a <main\+0x9a> bfb8      	it	lt
0000009c <main\+0x9c> 2000      	movlt	r0, #0
0000009e <main\+0x9e> 4348      	muls	r0, r1
000000a0 <main\+0xa0> bfb8      	it	lt
000000a2 <main\+0xa2> 2000      	movlt	r0, #0
000000a4 <main\+0xa4> 4348      	muls	r0, r1
