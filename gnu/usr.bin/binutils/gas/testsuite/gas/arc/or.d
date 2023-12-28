#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2105 0080           	or	r0,r1,r2
0x[0-9a-f]+ 2305 371a           	or	gp,fp,sp
0x[0-9a-f]+ 2605 37dd           	or	ilink,r30,blink
0x[0-9a-f]+ 2145 0000           	or	r0,r1,0
0x[0-9a-f]+ 2605 7080 0000 0000 	or	r0,0,r2
0x[0-9a-f]+ 2105 00be           	or	0,r1,r2
0x[0-9a-f]+ 2105 0f80 ffff ffff 	or	r0,r1,0xffffffff
0x[0-9a-f]+ 2605 7080 ffff ffff 	or	r0,0xffffffff,r2
0x[0-9a-f]+ 2105 0f80 0000 00ff 	or	r0,r1,0xff
0x[0-9a-f]+ 2605 7080 0000 00ff 	or	r0,0xff,r2
0x[0-9a-f]+ 2105 0f80 ffff ff00 	or	r0,r1,0xffffff00
0x[0-9a-f]+ 2605 7080 ffff ff00 	or	r0,0xffffff00,r2
0x[0-9a-f]+ 2105 0f80 0000 0100 	or	r0,r1,0x100
0x[0-9a-f]+ 2605 7080 ffff feff 	or	r0,0xfffffeff,r2
0x[0-9a-f]+ 2605 7f80 0000 0100 	or	r0,0x100,0x100
0x[0-9a-f]+ 2105 0f80 0000 0000 	or	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c5 0080           	or	r0,r0,r2
0x[0-9a-f]+ 23c5 0140           	or	r3,r3,r5
0x[0-9a-f]+ 26c5 0201           	or.eq	r6,r6,r8
0x[0-9a-f]+ 21c5 12c1           	or.eq	r9,r9,r11
0x[0-9a-f]+ 24c5 1382           	or.ne	r12,r12,r14
0x[0-9a-f]+ 27c5 1442           	or.ne	r15,r15,r17
0x[0-9a-f]+ 22c5 2503           	or.p	r18,r18,r20
0x[0-9a-f]+ 25c5 25c3           	or.p	r21,r21,r23
0x[0-9a-f]+ 20c5 3684           	or.n	r24,r24,gp
0x[0-9a-f]+ 23c5 3744           	or.n	fp,fp,ilink
0x[0-9a-f]+ 26c5 37c5           	or.c	r30,r30,blink
0x[0-9a-f]+ 23c5 00c5           	or.c	r3,r3,r3
0x[0-9a-f]+ 23c5 0205           	or.c	r3,r3,r8
0x[0-9a-f]+ 23c5 0106           	or.nc	r3,r3,r4
0x[0-9a-f]+ 24c5 0106           	or.nc	r4,r4,r4
0x[0-9a-f]+ 24c5 01c6           	or.nc	r4,r4,r7
0x[0-9a-f]+ 24c5 0147           	or.v	r4,r4,r5
0x[0-9a-f]+ 25c5 0147           	or.v	r5,r5,r5
0x[0-9a-f]+ 25c5 0148           	or.nv	r5,r5,r5
0x[0-9a-f]+ 25c5 0148           	or.nv	r5,r5,r5
0x[0-9a-f]+ 26c5 0009           	or.gt	r6,r6,r0
0x[0-9a-f]+ 20c5 002a           	or.ge	r0,r0,0
0x[0-9a-f]+ 21c5 006b           	or.lt	r1,r1,0x1
0x[0-9a-f]+ 23c5 00ed           	or.hi	r3,r3,0x3
0x[0-9a-f]+ 24c5 012e           	or.ls	r4,r4,0x4
0x[0-9a-f]+ 25c5 016f           	or.pnz	r5,r5,0x5
0x[0-9a-f]+ 2105 8080           	or.f	r0,r1,r2
0x[0-9a-f]+ 2145 8040           	or.f	r0,r1,0x1
0x[0-9a-f]+ 2605 f080 0000 0001 	or.f	r0,0x1,r2
0x[0-9a-f]+ 2105 80be           	or.f	0,r1,r2
0x[0-9a-f]+ 2105 8f80 0000 0200 	or.f	r0,r1,0x200
0x[0-9a-f]+ 2605 f080 0000 0200 	or.f	r0,0x200,r2
0x[0-9a-f]+ 21c5 8081           	or.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c5 8022           	or.f.ne	r0,r0,0
0x[0-9a-f]+ 22c5 808b           	or.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c5 f0a9 0000 0001 	or.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c5 ff8c 0000 0200 	or.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c5 f0aa 0000 0200 	or.f.ge	0,0x200,0x2
