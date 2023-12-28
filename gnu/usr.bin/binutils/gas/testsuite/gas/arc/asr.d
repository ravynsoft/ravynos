#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2902 0080           	asr	r0,r1,r2
0x[0-9a-f]+ 2b02 371a           	asr	gp,fp,sp
0x[0-9a-f]+ 2e02 37dd           	asr	ilink,r30,blink
0x[0-9a-f]+ 2942 0000           	asr	r0,r1,0
0x[0-9a-f]+ 2e02 7080 0000 0000 	asr	r0,0,r2
0x[0-9a-f]+ 2902 00be           	asr	0,r1,r2
0x[0-9a-f]+ 2902 0f80 ffff ffff 	asr	r0,r1,0xffffffff
0x[0-9a-f]+ 2e02 7080 ffff ffff 	asr	r0,0xffffffff,r2
0x[0-9a-f]+ 2902 0f80 0000 00ff 	asr	r0,r1,0xff
0x[0-9a-f]+ 2e02 7080 0000 00ff 	asr	r0,0xff,r2
0x[0-9a-f]+ 2902 0f80 ffff ff00 	asr	r0,r1,0xffffff00
0x[0-9a-f]+ 2e02 7080 ffff ff00 	asr	r0,0xffffff00,r2
0x[0-9a-f]+ 2902 0f80 0000 0100 	asr	r0,r1,0x100
0x[0-9a-f]+ 2e02 7080 ffff feff 	asr	r0,0xfffffeff,r2
0x[0-9a-f]+ 2e02 7f80 0000 0100 	asr	r0,0x100,0x100
0x[0-9a-f]+ 2902 0f80 0000 0000 	asr	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 28c2 0080           	asr	r0,r0,r2
0x[0-9a-f]+ 2bc2 0140           	asr	r3,r3,r5
0x[0-9a-f]+ 2ec2 0201           	asr.eq	r6,r6,r8
0x[0-9a-f]+ 29c2 12c1           	asr.eq	r9,r9,r11
0x[0-9a-f]+ 2cc2 1382           	asr.ne	r12,r12,r14
0x[0-9a-f]+ 2fc2 1442           	asr.ne	r15,r15,r17
0x[0-9a-f]+ 2ac2 2503           	asr.p	r18,r18,r20
0x[0-9a-f]+ 2dc2 25c3           	asr.p	r21,r21,r23
0x[0-9a-f]+ 28c2 3684           	asr.n	r24,r24,gp
0x[0-9a-f]+ 2bc2 3744           	asr.n	fp,fp,ilink
0x[0-9a-f]+ 2ec2 37c5           	asr.c	r30,r30,blink
0x[0-9a-f]+ 2bc2 00c5           	asr.c	r3,r3,r3
0x[0-9a-f]+ 2bc2 0205           	asr.c	r3,r3,r8
0x[0-9a-f]+ 2bc2 0106           	asr.nc	r3,r3,r4
0x[0-9a-f]+ 2cc2 0106           	asr.nc	r4,r4,r4
0x[0-9a-f]+ 2cc2 01c6           	asr.nc	r4,r4,r7
0x[0-9a-f]+ 2cc2 0147           	asr.v	r4,r4,r5
0x[0-9a-f]+ 2dc2 0147           	asr.v	r5,r5,r5
0x[0-9a-f]+ 2dc2 0148           	asr.nv	r5,r5,r5
0x[0-9a-f]+ 2dc2 0148           	asr.nv	r5,r5,r5
0x[0-9a-f]+ 2ec2 0009           	asr.gt	r6,r6,r0
0x[0-9a-f]+ 28c2 002a           	asr.ge	r0,r0,0
0x[0-9a-f]+ 29c2 006b           	asr.lt	r1,r1,0x1
0x[0-9a-f]+ 2bc2 00ed           	asr.hi	r3,r3,0x3
0x[0-9a-f]+ 2cc2 012e           	asr.ls	r4,r4,0x4
0x[0-9a-f]+ 2dc2 016f           	asr.pnz	r5,r5,0x5
0x[0-9a-f]+ 2902 8080           	asr.f	r0,r1,r2
0x[0-9a-f]+ 2942 8040           	asr.f	r0,r1,0x1
0x[0-9a-f]+ 2e02 f080 0000 0001 	asr.f	r0,0x1,r2
0x[0-9a-f]+ 2902 80be           	asr.f	0,r1,r2
0x[0-9a-f]+ 2902 8f80 0000 0200 	asr.f	r0,r1,0x200
0x[0-9a-f]+ 2e02 f080 0000 0200 	asr.f	r0,0x200,r2
0x[0-9a-f]+ 29c2 8081           	asr.f.eq	r1,r1,r2
0x[0-9a-f]+ 28c2 8022           	asr.f.ne	r0,r0,0
0x[0-9a-f]+ 2ac2 808b           	asr.f.lt	r2,r2,r2
0x[0-9a-f]+ 2ec2 f0a9 0000 0001 	asr.f.gt	0,0x1,0x2
0x[0-9a-f]+ 2ec2 ff8c 0000 0200 	asr.f.le	0,0x200,0x200
0x[0-9a-f]+ 2ec2 f0aa 0000 0200 	asr.f.ge	0,0x200,0x2
