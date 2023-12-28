#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2102 0080           	sub	r0,r1,r2
0x[0-9a-f]+ 2302 371a           	sub	gp,fp,sp
0x[0-9a-f]+ 2602 37dd           	sub	ilink,r30,blink
0x[0-9a-f]+ 2142 0000           	sub	r0,r1,0
0x[0-9a-f]+ 2602 7080 0000 0000 	sub	r0,0,r2
0x[0-9a-f]+ 2102 00be           	sub	0,r1,r2
0x[0-9a-f]+ 2102 0f80 ffff ffff 	sub	r0,r1,0xffffffff
0x[0-9a-f]+ 2602 7080 ffff ffff 	sub	r0,0xffffffff,r2
0x[0-9a-f]+ 2102 0f80 0000 00ff 	sub	r0,r1,0xff
0x[0-9a-f]+ 2602 7080 0000 00ff 	sub	r0,0xff,r2
0x[0-9a-f]+ 2102 0f80 ffff ff00 	sub	r0,r1,0xffffff00
0x[0-9a-f]+ 2602 7080 ffff ff00 	sub	r0,0xffffff00,r2
0x[0-9a-f]+ 2102 0f80 0000 0100 	sub	r0,r1,0x100
0x[0-9a-f]+ 2602 7080 ffff feff 	sub	r0,0xfffffeff,r2
0x[0-9a-f]+ 2602 7f80 0000 0100 	sub	r0,0x100,0x100
0x[0-9a-f]+ 2102 0f80 0000 0000 	sub	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c2 0080           	sub	r0,r0,r2
0x[0-9a-f]+ 23c2 0140           	sub	r3,r3,r5
0x[0-9a-f]+ 26c2 0201           	sub.eq	r6,r6,r8
0x[0-9a-f]+ 21c2 12c1           	sub.eq	r9,r9,r11
0x[0-9a-f]+ 24c2 1382           	sub.ne	r12,r12,r14
0x[0-9a-f]+ 27c2 1442           	sub.ne	r15,r15,r17
0x[0-9a-f]+ 22c2 2503           	sub.p	r18,r18,r20
0x[0-9a-f]+ 25c2 25c3           	sub.p	r21,r21,r23
0x[0-9a-f]+ 20c2 3684           	sub.n	r24,r24,gp
0x[0-9a-f]+ 23c2 3744           	sub.n	fp,fp,ilink
0x[0-9a-f]+ 26c2 37c5           	sub.c	r30,r30,blink
0x[0-9a-f]+ 23c2 00c5           	sub.c	r3,r3,r3
0x[0-9a-f]+ 23c2 0205           	sub.c	r3,r3,r8
0x[0-9a-f]+ 23c2 0106           	sub.nc	r3,r3,r4
0x[0-9a-f]+ 24c2 0106           	sub.nc	r4,r4,r4
0x[0-9a-f]+ 24c2 01c6           	sub.nc	r4,r4,r7
0x[0-9a-f]+ 24c2 0147           	sub.v	r4,r4,r5
0x[0-9a-f]+ 25c2 0147           	sub.v	r5,r5,r5
0x[0-9a-f]+ 25c2 0148           	sub.nv	r5,r5,r5
0x[0-9a-f]+ 25c2 0148           	sub.nv	r5,r5,r5
0x[0-9a-f]+ 26c2 0009           	sub.gt	r6,r6,r0
0x[0-9a-f]+ 20c2 002a           	sub.ge	r0,r0,0
0x[0-9a-f]+ 21c2 006b           	sub.lt	r1,r1,0x1
0x[0-9a-f]+ 23c2 00ed           	sub.hi	r3,r3,0x3
0x[0-9a-f]+ 24c2 012e           	sub.ls	r4,r4,0x4
0x[0-9a-f]+ 25c2 016f           	sub.pnz	r5,r5,0x5
0x[0-9a-f]+ 2102 8080           	sub.f	r0,r1,r2
0x[0-9a-f]+ 2142 8040           	sub.f	r0,r1,0x1
0x[0-9a-f]+ 2602 f080 0000 0001 	sub.f	r0,0x1,r2
0x[0-9a-f]+ 2102 80be           	sub.f	0,r1,r2
0x[0-9a-f]+ 2102 8f80 0000 0200 	sub.f	r0,r1,0x200
0x[0-9a-f]+ 2602 f080 0000 0200 	sub.f	r0,0x200,r2
0x[0-9a-f]+ 21c2 8081           	sub.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c2 8022           	sub.f.ne	r0,r0,0
0x[0-9a-f]+ 22c2 808b           	sub.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c2 f0a9 0000 0001 	sub.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c2 ff8c 0000 0200 	sub.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c2 f0aa 0000 0200 	sub.f.ge	0,0x200,0x2
