#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2903 0080           	ror	r0,r1,r2
0x[0-9a-f]+ 2b03 371a           	ror	gp,fp,sp
0x[0-9a-f]+ 2e03 37dd           	ror	ilink,r30,blink
0x[0-9a-f]+ 2943 0000           	ror	r0,r1,0
0x[0-9a-f]+ 2e03 7080 0000 0000 	ror	r0,0,r2
0x[0-9a-f]+ 2903 00be           	ror	0,r1,r2
0x[0-9a-f]+ 2903 0f80 ffff ffff 	ror	r0,r1,0xffffffff
0x[0-9a-f]+ 2e03 7080 ffff ffff 	ror	r0,0xffffffff,r2
0x[0-9a-f]+ 2903 0f80 0000 00ff 	ror	r0,r1,0xff
0x[0-9a-f]+ 2e03 7080 0000 00ff 	ror	r0,0xff,r2
0x[0-9a-f]+ 2903 0f80 ffff ff00 	ror	r0,r1,0xffffff00
0x[0-9a-f]+ 2e03 7080 ffff ff00 	ror	r0,0xffffff00,r2
0x[0-9a-f]+ 2903 0f80 0000 0100 	ror	r0,r1,0x100
0x[0-9a-f]+ 2e03 7080 ffff feff 	ror	r0,0xfffffeff,r2
0x[0-9a-f]+ 2e03 7f80 0000 0100 	ror	r0,0x100,0x100
0x[0-9a-f]+ 2903 0f80 0000 0000 	ror	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 28c3 0080           	ror	r0,r0,r2
0x[0-9a-f]+ 2bc3 0140           	ror	r3,r3,r5
0x[0-9a-f]+ 2ec3 0201           	ror.eq	r6,r6,r8
0x[0-9a-f]+ 29c3 12c1           	ror.eq	r9,r9,r11
0x[0-9a-f]+ 2cc3 1382           	ror.ne	r12,r12,r14
0x[0-9a-f]+ 2fc3 1442           	ror.ne	r15,r15,r17
0x[0-9a-f]+ 2ac3 2503           	ror.p	r18,r18,r20
0x[0-9a-f]+ 2dc3 25c3           	ror.p	r21,r21,r23
0x[0-9a-f]+ 28c3 3684           	ror.n	r24,r24,gp
0x[0-9a-f]+ 2bc3 3744           	ror.n	fp,fp,ilink
0x[0-9a-f]+ 2ec3 37c5           	ror.c	r30,r30,blink
0x[0-9a-f]+ 2bc3 00c5           	ror.c	r3,r3,r3
0x[0-9a-f]+ 2bc3 0205           	ror.c	r3,r3,r8
0x[0-9a-f]+ 2bc3 0106           	ror.nc	r3,r3,r4
0x[0-9a-f]+ 2cc3 0106           	ror.nc	r4,r4,r4
0x[0-9a-f]+ 2cc3 01c6           	ror.nc	r4,r4,r7
0x[0-9a-f]+ 2cc3 0147           	ror.v	r4,r4,r5
0x[0-9a-f]+ 2dc3 0147           	ror.v	r5,r5,r5
0x[0-9a-f]+ 2dc3 0148           	ror.nv	r5,r5,r5
0x[0-9a-f]+ 2dc3 0148           	ror.nv	r5,r5,r5
0x[0-9a-f]+ 2ec3 0009           	ror.gt	r6,r6,r0
0x[0-9a-f]+ 28c3 002a           	ror.ge	r0,r0,0
0x[0-9a-f]+ 29c3 006b           	ror.lt	r1,r1,0x1
0x[0-9a-f]+ 2bc3 00ed           	ror.hi	r3,r3,0x3
0x[0-9a-f]+ 2cc3 012e           	ror.ls	r4,r4,0x4
0x[0-9a-f]+ 2dc3 016f           	ror.pnz	r5,r5,0x5
0x[0-9a-f]+ 2903 8080           	ror.f	r0,r1,r2
0x[0-9a-f]+ 2943 8040           	ror.f	r0,r1,0x1
0x[0-9a-f]+ 2e03 f080 0000 0001 	ror.f	r0,0x1,r2
0x[0-9a-f]+ 2903 80be           	ror.f	0,r1,r2
0x[0-9a-f]+ 2903 8f80 0000 0200 	ror.f	r0,r1,0x200
0x[0-9a-f]+ 2e03 f080 0000 0200 	ror.f	r0,0x200,r2
0x[0-9a-f]+ 29c3 8081           	ror.f.eq	r1,r1,r2
0x[0-9a-f]+ 28c3 8022           	ror.f.ne	r0,r0,0
0x[0-9a-f]+ 2ac3 808b           	ror.f.lt	r2,r2,r2
0x[0-9a-f]+ 2ec3 f0a9 0000 0001 	ror.f.gt	0,0x1,0x2
0x[0-9a-f]+ 2ec3 ff8c 0000 0200 	ror.f.le	0,0x200,0x200
0x[0-9a-f]+ 2ec3 f0aa 0000 0200 	ror.f.ge	0,0x200,0x2
