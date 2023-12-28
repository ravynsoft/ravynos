#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2107 0080           	xor	r0,r1,r2
0x[0-9a-f]+ 2307 371a           	xor	gp,fp,sp
0x[0-9a-f]+ 2607 37dd           	xor	ilink,r30,blink
0x[0-9a-f]+ 2147 0000           	xor	r0,r1,0
0x[0-9a-f]+ 2607 7080 0000 0000 	xor	r0,0,r2
0x[0-9a-f]+ 2107 00be           	xor	0,r1,r2
0x[0-9a-f]+ 2107 0f80 ffff ffff 	xor	r0,r1,0xffffffff
0x[0-9a-f]+ 2607 7080 ffff ffff 	xor	r0,0xffffffff,r2
0x[0-9a-f]+ 2107 0f80 0000 00ff 	xor	r0,r1,0xff
0x[0-9a-f]+ 2607 7080 0000 00ff 	xor	r0,0xff,r2
0x[0-9a-f]+ 2107 0f80 ffff ff00 	xor	r0,r1,0xffffff00
0x[0-9a-f]+ 2607 7080 ffff ff00 	xor	r0,0xffffff00,r2
0x[0-9a-f]+ 2107 0f80 0000 0100 	xor	r0,r1,0x100
0x[0-9a-f]+ 2607 7080 ffff feff 	xor	r0,0xfffffeff,r2
0x[0-9a-f]+ 2607 7f80 0000 0100 	xor	r0,0x100,0x100
0x[0-9a-f]+ 2107 0f80 0000 0000 	xor	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c7 0080           	xor	r0,r0,r2
0x[0-9a-f]+ 23c7 0140           	xor	r3,r3,r5
0x[0-9a-f]+ 26c7 0201           	xor.eq	r6,r6,r8
0x[0-9a-f]+ 21c7 12c1           	xor.eq	r9,r9,r11
0x[0-9a-f]+ 24c7 1382           	xor.ne	r12,r12,r14
0x[0-9a-f]+ 27c7 1442           	xor.ne	r15,r15,r17
0x[0-9a-f]+ 22c7 2503           	xor.p	r18,r18,r20
0x[0-9a-f]+ 25c7 25c3           	xor.p	r21,r21,r23
0x[0-9a-f]+ 20c7 3684           	xor.n	r24,r24,gp
0x[0-9a-f]+ 23c7 3744           	xor.n	fp,fp,ilink
0x[0-9a-f]+ 26c7 37c5           	xor.c	r30,r30,blink
0x[0-9a-f]+ 23c7 00c5           	xor.c	r3,r3,r3
0x[0-9a-f]+ 23c7 0205           	xor.c	r3,r3,r8
0x[0-9a-f]+ 23c7 0106           	xor.nc	r3,r3,r4
0x[0-9a-f]+ 24c7 0106           	xor.nc	r4,r4,r4
0x[0-9a-f]+ 24c7 01c6           	xor.nc	r4,r4,r7
0x[0-9a-f]+ 24c7 0147           	xor.v	r4,r4,r5
0x[0-9a-f]+ 25c7 0147           	xor.v	r5,r5,r5
0x[0-9a-f]+ 25c7 0148           	xor.nv	r5,r5,r5
0x[0-9a-f]+ 25c7 0148           	xor.nv	r5,r5,r5
0x[0-9a-f]+ 26c7 0009           	xor.gt	r6,r6,r0
0x[0-9a-f]+ 20c7 002a           	xor.ge	r0,r0,0
0x[0-9a-f]+ 21c7 006b           	xor.lt	r1,r1,0x1
0x[0-9a-f]+ 23c7 00ed           	xor.hi	r3,r3,0x3
0x[0-9a-f]+ 24c7 012e           	xor.ls	r4,r4,0x4
0x[0-9a-f]+ 25c7 016f           	xor.pnz	r5,r5,0x5
0x[0-9a-f]+ 2107 8080           	xor.f	r0,r1,r2
0x[0-9a-f]+ 2147 8040           	xor.f	r0,r1,0x1
0x[0-9a-f]+ 2607 f080 0000 0001 	xor.f	r0,0x1,r2
0x[0-9a-f]+ 2107 80be           	xor.f	0,r1,r2
0x[0-9a-f]+ 2107 8f80 0000 0200 	xor.f	r0,r1,0x200
0x[0-9a-f]+ 2607 f080 0000 0200 	xor.f	r0,0x200,r2
0x[0-9a-f]+ 21c7 8081           	xor.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c7 8022           	xor.f.ne	r0,r0,0
0x[0-9a-f]+ 22c7 808b           	xor.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c7 f0a9 0000 0001 	xor.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c7 ff8c 0000 0200 	xor.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c7 f0aa 0000 0200 	xor.f.ge	0,0x200,0x2
