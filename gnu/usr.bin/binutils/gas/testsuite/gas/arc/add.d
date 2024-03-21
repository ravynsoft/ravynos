#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2100 0080           	add	r0,r1,r2
0x[0-9a-f]+ 2300 371a           	add	gp,fp,sp
0x[0-9a-f]+ 2600 37dd           	add	ilink,r30,blink
0x[0-9a-f]+ 2140 0000           	add	r0,r1,0
0x[0-9a-f]+ 2600 7080 0000 0000 	add	r0,0,r2
0x[0-9a-f]+ 2100 00be           	add	0,r1,r2
0x[0-9a-f]+ 2100 0f80 ffff ffff 	add	r0,r1,0xffffffff
0x[0-9a-f]+ 2600 7080 ffff ffff 	add	r0,0xffffffff,r2
0x[0-9a-f]+ 2100 0f80 0000 00ff 	add	r0,r1,0xff
0x[0-9a-f]+ 2600 7080 0000 00ff 	add	r0,0xff,r2
0x[0-9a-f]+ 2100 0f80 ffff ff00 	add	r0,r1,0xffffff00
0x[0-9a-f]+ 2600 7080 ffff ff00 	add	r0,0xffffff00,r2
0x[0-9a-f]+ 2100 0f80 0000 0100 	add	r0,r1,0x100
0x[0-9a-f]+ 2600 7080 ffff feff 	add	r0,0xfffffeff,r2
0x[0-9a-f]+ 2600 7f80 0000 0100 	add	r0,0x100,0x100
0x[0-9a-f]+ 2100 0f80 0000 0000 	add	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c0 0080           	add	r0,r0,r2
0x[0-9a-f]+ 23c0 0140           	add	r3,r3,r5
0x[0-9a-f]+ 26c0 0201           	add.eq	r6,r6,r8
0x[0-9a-f]+ 21c0 12c1           	add.eq	r9,r9,r11
0x[0-9a-f]+ 24c0 1382           	add.ne	r12,r12,r14
0x[0-9a-f]+ 27c0 1442           	add.ne	r15,r15,r17
0x[0-9a-f]+ 22c0 2503           	add.p	r18,r18,r20
0x[0-9a-f]+ 25c0 25c3           	add.p	r21,r21,r23
0x[0-9a-f]+ 20c0 3684           	add.n	r24,r24,gp
0x[0-9a-f]+ 23c0 3744           	add.n	fp,fp,ilink
0x[0-9a-f]+ 26c0 37c5           	add.c	r30,r30,blink
0x[0-9a-f]+ 23c0 00c5           	add.c	r3,r3,r3
0x[0-9a-f]+ 23c0 0205           	add.c	r3,r3,r8
0x[0-9a-f]+ 23c0 0106           	add.nc	r3,r3,r4
0x[0-9a-f]+ 24c0 0106           	add.nc	r4,r4,r4
0x[0-9a-f]+ 24c0 01c6           	add.nc	r4,r4,r7
0x[0-9a-f]+ 24c0 0147           	add.v	r4,r4,r5
0x[0-9a-f]+ 25c0 0147           	add.v	r5,r5,r5
0x[0-9a-f]+ 25c0 0148           	add.nv	r5,r5,r5
0x[0-9a-f]+ 25c0 0148           	add.nv	r5,r5,r5
0x[0-9a-f]+ 26c0 0009           	add.gt	r6,r6,r0
0x[0-9a-f]+ 20c0 002a           	add.ge	r0,r0,0
0x[0-9a-f]+ 21c0 006b           	add.lt	r1,r1,0x1
0x[0-9a-f]+ 23c0 00ed           	add.hi	r3,r3,0x3
0x[0-9a-f]+ 24c0 012e           	add.ls	r4,r4,0x4
0x[0-9a-f]+ 25c0 016f           	add.pnz	r5,r5,0x5
0x[0-9a-f]+ 2100 8080           	add.f	r0,r1,r2
0x[0-9a-f]+ 2140 8040           	add.f	r0,r1,0x1
0x[0-9a-f]+ 2600 f080 0000 0001 	add.f	r0,0x1,r2
0x[0-9a-f]+ 2100 80be           	add.f	0,r1,r2
0x[0-9a-f]+ 2100 8f80 0000 0200 	add.f	r0,r1,0x200
0x[0-9a-f]+ 2600 f080 0000 0200 	add.f	r0,0x200,r2
0x[0-9a-f]+ 21c0 8081           	add.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c0 8022           	add.f.ne	r0,r0,0
0x[0-9a-f]+ 22c0 808b           	add.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c0 f0a9 0000 0001 	add.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c0 ff8c 0000 0200 	add.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c0 f0aa 0000 0200 	add.f.ge	0,0x200,0x2
