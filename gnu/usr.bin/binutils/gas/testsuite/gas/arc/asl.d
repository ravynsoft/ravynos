#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2900 0080           	asl	r0,r1,r2
0x[0-9a-f]+ 2b00 371a           	asl	gp,fp,sp
0x[0-9a-f]+ 2e00 37dd           	asl	ilink,r30,blink
0x[0-9a-f]+ 2940 0000           	asl	r0,r1,0
0x[0-9a-f]+ 2e00 7080 0000 0000 	asl	r0,0,r2
0x[0-9a-f]+ 2900 00be           	asl	0,r1,r2
0x[0-9a-f]+ 2900 0f80 ffff ffff 	asl	r0,r1,0xffffffff
0x[0-9a-f]+ 2e00 7080 ffff ffff 	asl	r0,0xffffffff,r2
0x[0-9a-f]+ 2900 0f80 0000 00ff 	asl	r0,r1,0xff
0x[0-9a-f]+ 2e00 7080 0000 00ff 	asl	r0,0xff,r2
0x[0-9a-f]+ 2900 0f80 ffff ff00 	asl	r0,r1,0xffffff00
0x[0-9a-f]+ 2e00 7080 ffff ff00 	asl	r0,0xffffff00,r2
0x[0-9a-f]+ 2900 0f80 0000 0100 	asl	r0,r1,0x100
0x[0-9a-f]+ 2e00 7080 ffff feff 	asl	r0,0xfffffeff,r2
0x[0-9a-f]+ 2e00 7f80 0000 0100 	asl	r0,0x100,0x100
0x[0-9a-f]+ 2900 0f80 0000 0000 	asl	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 28c0 0080           	asl	r0,r0,r2
0x[0-9a-f]+ 2bc0 0140           	asl	r3,r3,r5
0x[0-9a-f]+ 2ec0 0201           	asl.eq	r6,r6,r8
0x[0-9a-f]+ 29c0 12c1           	asl.eq	r9,r9,r11
0x[0-9a-f]+ 2cc0 1382           	asl.ne	r12,r12,r14
0x[0-9a-f]+ 2fc0 1442           	asl.ne	r15,r15,r17
0x[0-9a-f]+ 2ac0 2503           	asl.p	r18,r18,r20
0x[0-9a-f]+ 2dc0 25c3           	asl.p	r21,r21,r23
0x[0-9a-f]+ 28c0 3684           	asl.n	r24,r24,gp
0x[0-9a-f]+ 2bc0 3744           	asl.n	fp,fp,ilink
0x[0-9a-f]+ 2ec0 37c5           	asl.c	r30,r30,blink
0x[0-9a-f]+ 2bc0 00c5           	asl.c	r3,r3,r3
0x[0-9a-f]+ 2bc0 0205           	asl.c	r3,r3,r8
0x[0-9a-f]+ 2bc0 0106           	asl.nc	r3,r3,r4
0x[0-9a-f]+ 2cc0 0106           	asl.nc	r4,r4,r4
0x[0-9a-f]+ 2cc0 01c6           	asl.nc	r4,r4,r7
0x[0-9a-f]+ 2cc0 0147           	asl.v	r4,r4,r5
0x[0-9a-f]+ 2dc0 0147           	asl.v	r5,r5,r5
0x[0-9a-f]+ 2dc0 0148           	asl.nv	r5,r5,r5
0x[0-9a-f]+ 2dc0 0148           	asl.nv	r5,r5,r5
0x[0-9a-f]+ 2ec0 0009           	asl.gt	r6,r6,r0
0x[0-9a-f]+ 28c0 002a           	asl.ge	r0,r0,0
0x[0-9a-f]+ 29c0 006b           	asl.lt	r1,r1,0x1
0x[0-9a-f]+ 2bc0 00ed           	asl.hi	r3,r3,0x3
0x[0-9a-f]+ 2cc0 012e           	asl.ls	r4,r4,0x4
0x[0-9a-f]+ 2dc0 016f           	asl.pnz	r5,r5,0x5
0x[0-9a-f]+ 2900 8080           	asl.f	r0,r1,r2
0x[0-9a-f]+ 2940 8040           	asl.f	r0,r1,0x1
0x[0-9a-f]+ 2e00 f080 0000 0001 	asl.f	r0,0x1,r2
0x[0-9a-f]+ 2900 80be           	asl.f	0,r1,r2
0x[0-9a-f]+ 2900 8f80 0000 0200 	asl.f	r0,r1,0x200
0x[0-9a-f]+ 2e00 f080 0000 0200 	asl.f	r0,0x200,r2
0x[0-9a-f]+ 29c0 8081           	asl.f.eq	r1,r1,r2
0x[0-9a-f]+ 28c0 8022           	asl.f.ne	r0,r0,0
0x[0-9a-f]+ 2ac0 808b           	asl.f.lt	r2,r2,r2
0x[0-9a-f]+ 2ec0 f0a9 0000 0001 	asl.f.gt	0,0x1,0x2
0x[0-9a-f]+ 2ec0 ff8c 0000 0200 	asl.f.le	0,0x200,0x200
0x[0-9a-f]+ 2ec0 f0aa 0000 0200 	asl.f.ge	0,0x200,0x2
