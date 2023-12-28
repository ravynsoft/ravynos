#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2106 0080           	bic	r0,r1,r2
0x[0-9a-f]+ 2306 371a           	bic	gp,fp,sp
0x[0-9a-f]+ 2606 37dd           	bic	ilink,r30,blink
0x[0-9a-f]+ 2146 0000           	bic	r0,r1,0
0x[0-9a-f]+ 2606 7080 0000 0000 	bic	r0,0,r2
0x[0-9a-f]+ 2106 00be           	bic	0,r1,r2
0x[0-9a-f]+ 2106 0f80 ffff ffff 	bic	r0,r1,0xffffffff
0x[0-9a-f]+ 2606 7080 ffff ffff 	bic	r0,0xffffffff,r2
0x[0-9a-f]+ 2106 0f80 0000 00ff 	bic	r0,r1,0xff
0x[0-9a-f]+ 2606 7080 0000 00ff 	bic	r0,0xff,r2
0x[0-9a-f]+ 2106 0f80 ffff ff00 	bic	r0,r1,0xffffff00
0x[0-9a-f]+ 2606 7080 ffff ff00 	bic	r0,0xffffff00,r2
0x[0-9a-f]+ 2106 0f80 0000 0100 	bic	r0,r1,0x100
0x[0-9a-f]+ 2606 7080 ffff feff 	bic	r0,0xfffffeff,r2
0x[0-9a-f]+ 2606 7f80 0000 0100 	bic	r0,0x100,0x100
0x[0-9a-f]+ 2106 0f80 0000 0000 	bic	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c6 0080           	bic	r0,r0,r2
0x[0-9a-f]+ 23c6 0140           	bic	r3,r3,r5
0x[0-9a-f]+ 26c6 0201           	bic.eq	r6,r6,r8
0x[0-9a-f]+ 21c6 12c1           	bic.eq	r9,r9,r11
0x[0-9a-f]+ 24c6 1382           	bic.ne	r12,r12,r14
0x[0-9a-f]+ 27c6 1442           	bic.ne	r15,r15,r17
0x[0-9a-f]+ 22c6 2503           	bic.p	r18,r18,r20
0x[0-9a-f]+ 25c6 25c3           	bic.p	r21,r21,r23
0x[0-9a-f]+ 20c6 3684           	bic.n	r24,r24,gp
0x[0-9a-f]+ 23c6 3744           	bic.n	fp,fp,ilink
0x[0-9a-f]+ 26c6 37c5           	bic.c	r30,r30,blink
0x[0-9a-f]+ 23c6 00c5           	bic.c	r3,r3,r3
0x[0-9a-f]+ 23c6 0205           	bic.c	r3,r3,r8
0x[0-9a-f]+ 23c6 0106           	bic.nc	r3,r3,r4
0x[0-9a-f]+ 24c6 0106           	bic.nc	r4,r4,r4
0x[0-9a-f]+ 24c6 01c6           	bic.nc	r4,r4,r7
0x[0-9a-f]+ 24c6 0147           	bic.v	r4,r4,r5
0x[0-9a-f]+ 25c6 0147           	bic.v	r5,r5,r5
0x[0-9a-f]+ 25c6 0148           	bic.nv	r5,r5,r5
0x[0-9a-f]+ 25c6 0148           	bic.nv	r5,r5,r5
0x[0-9a-f]+ 26c6 0009           	bic.gt	r6,r6,r0
0x[0-9a-f]+ 20c6 002a           	bic.ge	r0,r0,0
0x[0-9a-f]+ 21c6 006b           	bic.lt	r1,r1,0x1
0x[0-9a-f]+ 23c6 00ed           	bic.hi	r3,r3,0x3
0x[0-9a-f]+ 24c6 012e           	bic.ls	r4,r4,0x4
0x[0-9a-f]+ 25c6 016f           	bic.pnz	r5,r5,0x5
0x[0-9a-f]+ 2106 8080           	bic.f	r0,r1,r2
0x[0-9a-f]+ 2146 8040           	bic.f	r0,r1,0x1
0x[0-9a-f]+ 2606 f080 0000 0001 	bic.f	r0,0x1,r2
0x[0-9a-f]+ 2106 80be           	bic.f	0,r1,r2
0x[0-9a-f]+ 2106 8f80 0000 0200 	bic.f	r0,r1,0x200
0x[0-9a-f]+ 2606 f080 0000 0200 	bic.f	r0,0x200,r2
0x[0-9a-f]+ 21c6 8081           	bic.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c6 8022           	bic.f.ne	r0,r0,0
0x[0-9a-f]+ 22c6 808b           	bic.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c6 f0a9 0000 0001 	bic.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c6 ff8c 0000 0200 	bic.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c6 f0aa 0000 0200 	bic.f.ge	0,0x200,0x2
