#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2901 0080           	lsr	r0,r1,r2
0x[0-9a-f]+ 2b01 371a           	lsr	gp,fp,sp
0x[0-9a-f]+ 2e01 37dd           	lsr	ilink,r30,blink
0x[0-9a-f]+ 2941 0000           	lsr	r0,r1,0
0x[0-9a-f]+ 2e01 7080 0000 0000 	lsr	r0,0,r2
0x[0-9a-f]+ 2901 00be           	lsr	0,r1,r2
0x[0-9a-f]+ 2901 0f80 ffff ffff 	lsr	r0,r1,0xffffffff
0x[0-9a-f]+ 2e01 7080 ffff ffff 	lsr	r0,0xffffffff,r2
0x[0-9a-f]+ 2901 0f80 0000 00ff 	lsr	r0,r1,0xff
0x[0-9a-f]+ 2e01 7080 0000 00ff 	lsr	r0,0xff,r2
0x[0-9a-f]+ 2901 0f80 ffff ff00 	lsr	r0,r1,0xffffff00
0x[0-9a-f]+ 2e01 7080 ffff ff00 	lsr	r0,0xffffff00,r2
0x[0-9a-f]+ 2901 0f80 0000 0100 	lsr	r0,r1,0x100
0x[0-9a-f]+ 2e01 7080 ffff feff 	lsr	r0,0xfffffeff,r2
0x[0-9a-f]+ 2e01 7f80 0000 0100 	lsr	r0,0x100,0x100
0x[0-9a-f]+ 2901 0f80 0000 0000 	lsr	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 28c1 0080           	lsr	r0,r0,r2
0x[0-9a-f]+ 2bc1 0140           	lsr	r3,r3,r5
0x[0-9a-f]+ 2ec1 0201           	lsr.eq	r6,r6,r8
0x[0-9a-f]+ 29c1 12c1           	lsr.eq	r9,r9,r11
0x[0-9a-f]+ 2cc1 1382           	lsr.ne	r12,r12,r14
0x[0-9a-f]+ 2fc1 1442           	lsr.ne	r15,r15,r17
0x[0-9a-f]+ 2ac1 2503           	lsr.p	r18,r18,r20
0x[0-9a-f]+ 2dc1 25c3           	lsr.p	r21,r21,r23
0x[0-9a-f]+ 28c1 3684           	lsr.n	r24,r24,gp
0x[0-9a-f]+ 2bc1 3744           	lsr.n	fp,fp,ilink
0x[0-9a-f]+ 2ec1 37c5           	lsr.c	r30,r30,blink
0x[0-9a-f]+ 2bc1 00c5           	lsr.c	r3,r3,r3
0x[0-9a-f]+ 2bc1 0205           	lsr.c	r3,r3,r8
0x[0-9a-f]+ 2bc1 0106           	lsr.nc	r3,r3,r4
0x[0-9a-f]+ 2cc1 0106           	lsr.nc	r4,r4,r4
0x[0-9a-f]+ 2cc1 01c6           	lsr.nc	r4,r4,r7
0x[0-9a-f]+ 2cc1 0147           	lsr.v	r4,r4,r5
0x[0-9a-f]+ 2dc1 0147           	lsr.v	r5,r5,r5
0x[0-9a-f]+ 2dc1 0148           	lsr.nv	r5,r5,r5
0x[0-9a-f]+ 2dc1 0148           	lsr.nv	r5,r5,r5
0x[0-9a-f]+ 2ec1 0009           	lsr.gt	r6,r6,r0
0x[0-9a-f]+ 28c1 002a           	lsr.ge	r0,r0,0
0x[0-9a-f]+ 29c1 006b           	lsr.lt	r1,r1,0x1
0x[0-9a-f]+ 2bc1 00ed           	lsr.hi	r3,r3,0x3
0x[0-9a-f]+ 2cc1 012e           	lsr.ls	r4,r4,0x4
0x[0-9a-f]+ 2dc1 016f           	lsr.pnz	r5,r5,0x5
0x[0-9a-f]+ 2901 8080           	lsr.f	r0,r1,r2
0x[0-9a-f]+ 2941 8040           	lsr.f	r0,r1,0x1
0x[0-9a-f]+ 2e01 f080 0000 0001 	lsr.f	r0,0x1,r2
0x[0-9a-f]+ 2901 80be           	lsr.f	0,r1,r2
0x[0-9a-f]+ 2901 8f80 0000 0200 	lsr.f	r0,r1,0x200
0x[0-9a-f]+ 2e01 f080 0000 0200 	lsr.f	r0,0x200,r2
0x[0-9a-f]+ 29c1 8081           	lsr.f.eq	r1,r1,r2
0x[0-9a-f]+ 28c1 8022           	lsr.f.ne	r0,r0,0
0x[0-9a-f]+ 2ac1 808b           	lsr.f.lt	r2,r2,r2
0x[0-9a-f]+ 2ec1 f0a9 0000 0001 	lsr.f.gt	0,0x1,0x2
0x[0-9a-f]+ 2ec1 ff8c 0000 0200 	lsr.f.le	0,0x200,0x200
0x[0-9a-f]+ 2ec1 f0aa 0000 0200 	lsr.f.ge	0,0x200,0x2
