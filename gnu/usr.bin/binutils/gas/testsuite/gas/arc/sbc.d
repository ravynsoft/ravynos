#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2103 0080           	sbc	r0,r1,r2
0x[0-9a-f]+ 2303 371a           	sbc	gp,fp,sp
0x[0-9a-f]+ 2603 37dd           	sbc	ilink,r30,blink
0x[0-9a-f]+ 2143 0000           	sbc	r0,r1,0
0x[0-9a-f]+ 2603 7080 0000 0000 	sbc	r0,0,r2
0x[0-9a-f]+ 2103 00be           	sbc	0,r1,r2
0x[0-9a-f]+ 2103 0f80 ffff ffff 	sbc	r0,r1,0xffffffff
0x[0-9a-f]+ 2603 7080 ffff ffff 	sbc	r0,0xffffffff,r2
0x[0-9a-f]+ 2103 0f80 0000 00ff 	sbc	r0,r1,0xff
0x[0-9a-f]+ 2603 7080 0000 00ff 	sbc	r0,0xff,r2
0x[0-9a-f]+ 2103 0f80 ffff ff00 	sbc	r0,r1,0xffffff00
0x[0-9a-f]+ 2603 7080 ffff ff00 	sbc	r0,0xffffff00,r2
0x[0-9a-f]+ 2103 0f80 0000 0100 	sbc	r0,r1,0x100
0x[0-9a-f]+ 2603 7080 ffff feff 	sbc	r0,0xfffffeff,r2
0x[0-9a-f]+ 2603 7f80 0000 0100 	sbc	r0,0x100,0x100
0x[0-9a-f]+ 2103 0f80 0000 0000 	sbc	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c3 0080           	sbc	r0,r0,r2
0x[0-9a-f]+ 23c3 0140           	sbc	r3,r3,r5
0x[0-9a-f]+ 26c3 0201           	sbc.eq	r6,r6,r8
0x[0-9a-f]+ 21c3 12c1           	sbc.eq	r9,r9,r11
0x[0-9a-f]+ 24c3 1382           	sbc.ne	r12,r12,r14
0x[0-9a-f]+ 27c3 1442           	sbc.ne	r15,r15,r17
0x[0-9a-f]+ 22c3 2503           	sbc.p	r18,r18,r20
0x[0-9a-f]+ 25c3 25c3           	sbc.p	r21,r21,r23
0x[0-9a-f]+ 20c3 3684           	sbc.n	r24,r24,gp
0x[0-9a-f]+ 23c3 3744           	sbc.n	fp,fp,ilink
0x[0-9a-f]+ 26c3 37c5           	sbc.c	r30,r30,blink
0x[0-9a-f]+ 23c3 00c5           	sbc.c	r3,r3,r3
0x[0-9a-f]+ 23c3 0205           	sbc.c	r3,r3,r8
0x[0-9a-f]+ 23c3 0106           	sbc.nc	r3,r3,r4
0x[0-9a-f]+ 24c3 0106           	sbc.nc	r4,r4,r4
0x[0-9a-f]+ 24c3 01c6           	sbc.nc	r4,r4,r7
0x[0-9a-f]+ 24c3 0147           	sbc.v	r4,r4,r5
0x[0-9a-f]+ 25c3 0147           	sbc.v	r5,r5,r5
0x[0-9a-f]+ 25c3 0148           	sbc.nv	r5,r5,r5
0x[0-9a-f]+ 25c3 0148           	sbc.nv	r5,r5,r5
0x[0-9a-f]+ 26c3 0009           	sbc.gt	r6,r6,r0
0x[0-9a-f]+ 20c3 002a           	sbc.ge	r0,r0,0
0x[0-9a-f]+ 21c3 006b           	sbc.lt	r1,r1,0x1
0x[0-9a-f]+ 23c3 00ed           	sbc.hi	r3,r3,0x3
0x[0-9a-f]+ 24c3 012e           	sbc.ls	r4,r4,0x4
0x[0-9a-f]+ 25c3 016f           	sbc.pnz	r5,r5,0x5
0x[0-9a-f]+ 2103 8080           	sbc.f	r0,r1,r2
0x[0-9a-f]+ 2143 8040           	sbc.f	r0,r1,0x1
0x[0-9a-f]+ 2603 f080 0000 0001 	sbc.f	r0,0x1,r2
0x[0-9a-f]+ 2103 80be           	sbc.f	0,r1,r2
0x[0-9a-f]+ 2103 8f80 0000 0200 	sbc.f	r0,r1,0x200
0x[0-9a-f]+ 2603 f080 0000 0200 	sbc.f	r0,0x200,r2
0x[0-9a-f]+ 21c3 8081           	sbc.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c3 8022           	sbc.f.ne	r0,r0,0
0x[0-9a-f]+ 22c3 808b           	sbc.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c3 f0a9 0000 0001 	sbc.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c3 ff8c 0000 0200 	sbc.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c3 f0aa 0000 0200 	sbc.f.ge	0,0x200,0x2
