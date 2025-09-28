# name: SP and PC registers special uses test.
# objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <foo> 4685      	mov	sp, r0
00000002 <foo\+0x2> 4668      	mov	r0, sp
00000004 <foo\+0x4> b000      	add	sp, #0
00000006 <foo\+0x6> f20d 0d00 	addw	sp, sp, #0
0000000a <foo\+0xa> b080      	sub	sp, #0
0000000c <foo\+0xc> f2ad 0d00 	subw	sp, sp, #0
00000010 <foo\+0x10> 4485      	add	sp, r0
00000012 <foo\+0x12> eb0d 0d40 	add.w	sp, sp, r0, lsl #1
00000016 <foo\+0x16> ebad 0d00 	sub.w	sp, sp, r0
0000001a <foo\+0x1a> ebad 0d40 	sub.w	sp, sp, r0, lsl #1
0000001e <foo\+0x1e> 9800      	ldr	r0, \[sp, #0\]
00000020 <foo\+0x20> 4800      	ldr	r0, \[pc, #0\]	@ \(00000024 <foo\+0x24>\)
00000022 <foo\+0x22> f8d0 f000 	ldr.w	pc, \[r0\]
00000026 <foo\+0x26> f8d0 d000 	ldr.w	sp, \[r0\]
0000002a <foo\+0x2a> f8df f000 	ldr.w	pc, \[pc\]	@ 0000002c <foo\+0x2c>
0000002e <foo\+0x2e> f8dd d000 	ldr.w	sp, \[sp\]
00000032 <foo\+0x32> f8dd f000 	ldr.w	pc, \[sp\]
00000036 <foo\+0x36> f8df d000 	ldr.w	sp, \[pc\]	@ 00000038 <foo\+0x38>
0000003a <foo\+0x3a> 9000      	str	r0, \[sp, #0\]
0000003c <foo\+0x3c> f8c0 d000 	str.w	sp, \[r0\]
00000040 <foo\+0x40> f8cd d000 	str.w	sp, \[sp\]
00000044 <foo\+0x44> 4468      	add	r0, sp
00000046 <foo\+0x46> eb1d 0000 	adds.w	r0, sp, r0
0000004a <foo\+0x4a> eb0d 0040 	add.w	r0, sp, r0, lsl #1
0000004e <foo\+0x4e> eb1d 0040 	adds.w	r0, sp, r0, lsl #1
00000052 <foo\+0x52> f11d 0f00 	cmn.w	sp, #0
00000056 <foo\+0x56> eb1d 0f00 	cmn.w	sp, r0
0000005a <foo\+0x5a> eb1d 0f40 	cmn.w	sp, r0, lsl #1
0000005e <foo\+0x5e> f1bd 0f00 	cmp.w	sp, #0
00000062 <foo\+0x62> 4585      	cmp	sp, r0
00000064 <foo\+0x64> ebbd 0f40 	cmp.w	sp, r0, lsl #1
00000068 <foo\+0x68> b080      	sub	sp, #0
0000006a <foo\+0x6a> f1bd 0d00 	subs.w	sp, sp, #0
0000006e <foo\+0x6e> f1ad 0000 	sub.w	r0, sp, #0
00000072 <foo\+0x72> f1bd 0000 	subs.w	r0, sp, #0
00000076 <foo\+0x76> b001      	add	sp, #4
00000078 <foo\+0x78> a801      	add	r0, sp, #4
0000007a <foo\+0x7a> f11d 0d04 	adds.w	sp, sp, #4
0000007e <foo\+0x7e> f11d 0004 	adds.w	r0, sp, #4
00000082 <foo\+0x82> f20d 0004 	addw	r0, sp, #4
00000086 <foo\+0x86> b001      	add	sp, #4
00000088 <foo\+0x88> f11d 0d04 	adds.w	sp, sp, #4
0000008c <foo\+0x8c> f20d 0d04 	addw	sp, sp, #4
00000090 <foo\+0x90> 4485      	add	sp, r0
00000092 <foo\+0x92> 4468      	add	r0, sp
00000094 <foo\+0x94> eb0d 0040 	add.w	r0, sp, r0, lsl #1
00000098 <foo\+0x98> eb1d 0d00 	adds.w	sp, sp, r0
0000009c <foo\+0x9c> eb1d 0000 	adds.w	r0, sp, r0
000000a0 <foo\+0xa0> eb1d 0040 	adds.w	r0, sp, r0, lsl #1
000000a4 <foo\+0xa4> 4485      	add	sp, r0
000000a6 <foo\+0xa6> eb0d 0d40 	add.w	sp, sp, r0, lsl #1
000000aa <foo\+0xaa> eb1d 0d00 	adds.w	sp, sp, r0
000000ae <foo\+0xae> eb1d 0d40 	adds.w	sp, sp, r0, lsl #1
000000b2 <foo\+0xb2> 44ed      	add	sp, sp
000000b4 <foo\+0xb4> f1ad 0000 	sub.w	r0, sp, #0
000000b8 <foo\+0xb8> f1bd 0000 	subs.w	r0, sp, #0
000000bc <foo\+0xbc> f2ad 0000 	subw	r0, sp, #0
000000c0 <foo\+0xc0> b080      	sub	sp, #0
000000c2 <foo\+0xc2> f1bd 0d00 	subs.w	sp, sp, #0
000000c6 <foo\+0xc6> f2ad 0d00 	subw	sp, sp, #0
000000ca <foo\+0xca> b080      	sub	sp, #0
000000cc <foo\+0xcc> f1bd 0d00 	subs.w	sp, sp, #0
000000d0 <foo\+0xd0> ebad 0040 	sub.w	r0, sp, r0, lsl #1
000000d4 <foo\+0xd4> ebbd 0040 	subs.w	r0, sp, r0, lsl #1
000000d8 <foo\+0xd8> ebad 0d40 	sub.w	sp, sp, r0, lsl #1
000000dc <foo\+0xdc> ebbd 0d40 	subs.w	sp, sp, r0, lsl #1
000000e0 <foo\+0xe0> a001      	add	r0, pc, #4	@ \(adr r0, 000000e8 <foo\+0xe8>\)
000000e2 <foo\+0xe2> f2af 0004 	subw	r0, pc, #4
000000e6 <foo\+0xe6> f20f 0004 	addw	r0, pc, #4
000000ea <foo\+0xea> f2af 0004 	subw	r0, pc, #4
000000ee <foo\+0xee> f20f 0004 	addw	r0, pc, #4
000000f2 <foo\+0xf2> f2af 0004 	subw	r0, pc, #4
000000f6 <foo\+0xf6> bf00      	nop
000000f8 <foo\+0xf8> bf00      	nop
000000fa <foo\+0xfa> bf00      	nop
#pass
