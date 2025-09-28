# name: ldr - thumb
#objdump: -dr --prefix-address --show-raw-insn

.*: +file format .*arm.*

Disassembly of section [^>]+:
0+00 <[^>]+> f8d1 1005 	ldr.w	r1, \[r1, #5\]
0+04 <[^>]+> f852 1f05 	ldr.w	r1, \[r2, #5\]!
0+08 <[^>]+> f8df 1005 	ldr.w	r1, \[pc, #5\]	@ 0+11 <[^>]+0x11>
0+0c <[^>]+> f8d1 f005 	ldr.w	pc, \[r1, #5\]
0+10 <[^>]+> f8df f004 	ldr.w	pc, \[pc, #4\]	@ 0+18 <[^>]+0x18>
0+14 <[^>]+> bfa2      	ittt	ge
0+16 <[^>]+> 4901      	ldrge	r1, \[pc, #4\]	@ \(0+1c <[^>]+0x1c>\)
0+18 <[^>]+> bf00      	nopge
0+1a <[^>]+> bf00      	nopge
0+1c <[^>]+> bfa8      	it	ge
0+1e <[^>]+> f8df f004 	ldrge.w	pc, \[pc, #4\]	@ 0+24 <[^>]+0x24>
0+22 <[^>]+> bfa2      	ittt	ge
0+24 <[^>]+> f85f 1ab8 	ldrge.w	r1, \[pc, #-2744\]	@ fffff570 <[^>]+>
0+28 <[^>]+> bf00      	nopge
0+2a <[^>]+> bf00      	nopge
0+2c <[^>]+> bfa8      	it	ge
0+2e <[^>]+> f85f fab6 	ldrge.w	pc, \[pc, #-2742\]	@ fffff57a <[^>]+>
0+32 <[^>]+> f85f 1ab9 	ldr.w	r1, \[pc, #-2745\]	@ fffff57b <[^>]+>
0+36 <[^>]+> f85f fab6 	ldr.w	pc, \[pc, #-2742\]	@ fffff582 <[^>]+>
0+3a <[^>]+> bfa2      	ittt	ge
0+3c <[^>]+> 5851      	ldrge	r1, \[r2, r1\]
0+3e <[^>]+> bf00      	nopge
0+40 <[^>]+> bf00      	nopge
0+42 <[^>]+> bfa8      	it	ge
0+44 <[^>]+> f852 f001 	ldrge.w	pc, \[r2, r1\]
0+48 <[^>]+> 58d1      	ldr	r1, \[r2, r3\]
0+4a <[^>]+> f8c2 100a 	str.w	r1, \[r2, #10\]
0+4e <[^>]+> f8c1 100a 	str.w	r1, \[r1, #10\]
0+52 <[^>]+> f842 1f0a 	str.w	r1, \[r2, #10\]!
0+56 <[^>]+> 50d1      	str	r1, \[r2, r3\]
#pass
