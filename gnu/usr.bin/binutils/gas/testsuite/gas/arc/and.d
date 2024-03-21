#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2104 0080           	and	r0,r1,r2
0x[0-9a-f]+ 2304 371a           	and	gp,fp,sp
0x[0-9a-f]+ 2604 37dd           	and	ilink,r30,blink
0x[0-9a-f]+ 2144 0000           	and	r0,r1,0
0x[0-9a-f]+ 2604 7080 0000 0000 	and	r0,0,r2
0x[0-9a-f]+ 2104 00be           	and	0,r1,r2
0x[0-9a-f]+ 2104 0f80 ffff ffff 	and	r0,r1,0xffffffff
0x[0-9a-f]+ 2604 7080 ffff ffff 	and	r0,0xffffffff,r2
0x[0-9a-f]+ 2104 0f80 0000 00ff 	and	r0,r1,0xff
0x[0-9a-f]+ 2604 7080 0000 00ff 	and	r0,0xff,r2
0x[0-9a-f]+ 2104 0f80 ffff ff00 	and	r0,r1,0xffffff00
0x[0-9a-f]+ 2604 7080 ffff ff00 	and	r0,0xffffff00,r2
0x[0-9a-f]+ 2104 0f80 0000 0100 	and	r0,r1,0x100
0x[0-9a-f]+ 2604 7080 ffff feff 	and	r0,0xfffffeff,r2
0x[0-9a-f]+ 2604 7f80 0000 0100 	and	r0,0x100,0x100
0x[0-9a-f]+ 2104 0f80 0000 0000 	and	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c4 0080           	and	r0,r0,r2
0x[0-9a-f]+ 23c4 0140           	and	r3,r3,r5
0x[0-9a-f]+ 26c4 0201           	and.eq	r6,r6,r8
0x[0-9a-f]+ 21c4 12c1           	and.eq	r9,r9,r11
0x[0-9a-f]+ 24c4 1382           	and.ne	r12,r12,r14
0x[0-9a-f]+ 27c4 1442           	and.ne	r15,r15,r17
0x[0-9a-f]+ 22c4 2503           	and.p	r18,r18,r20
0x[0-9a-f]+ 25c4 25c3           	and.p	r21,r21,r23
0x[0-9a-f]+ 20c4 3684           	and.n	r24,r24,gp
0x[0-9a-f]+ 23c4 3744           	and.n	fp,fp,ilink
0x[0-9a-f]+ 26c4 37c5           	and.c	r30,r30,blink
0x[0-9a-f]+ 23c4 00c5           	and.c	r3,r3,r3
0x[0-9a-f]+ 23c4 0205           	and.c	r3,r3,r8
0x[0-9a-f]+ 23c4 0106           	and.nc	r3,r3,r4
0x[0-9a-f]+ 24c4 0106           	and.nc	r4,r4,r4
0x[0-9a-f]+ 24c4 01c6           	and.nc	r4,r4,r7
0x[0-9a-f]+ 24c4 0147           	and.v	r4,r4,r5
0x[0-9a-f]+ 25c4 0147           	and.v	r5,r5,r5
0x[0-9a-f]+ 25c4 0148           	and.nv	r5,r5,r5
0x[0-9a-f]+ 25c4 0148           	and.nv	r5,r5,r5
0x[0-9a-f]+ 26c4 0009           	and.gt	r6,r6,r0
0x[0-9a-f]+ 20c4 002a           	and.ge	r0,r0,0
0x[0-9a-f]+ 21c4 006b           	and.lt	r1,r1,0x1
0x[0-9a-f]+ 23c4 00ed           	and.hi	r3,r3,0x3
0x[0-9a-f]+ 24c4 012e           	and.ls	r4,r4,0x4
0x[0-9a-f]+ 25c4 016f           	and.pnz	r5,r5,0x5
0x[0-9a-f]+ 2104 8080           	and.f	r0,r1,r2
0x[0-9a-f]+ 2144 8040           	and.f	r0,r1,0x1
0x[0-9a-f]+ 2604 f080 0000 0001 	and.f	r0,0x1,r2
0x[0-9a-f]+ 2104 80be           	and.f	0,r1,r2
0x[0-9a-f]+ 2104 8f80 0000 0200 	and.f	r0,r1,0x200
0x[0-9a-f]+ 2604 f080 0000 0200 	and.f	r0,0x200,r2
0x[0-9a-f]+ 21c4 8081           	and.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c4 8022           	and.f.ne	r0,r0,0
0x[0-9a-f]+ 22c4 808b           	and.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c4 f0a9 0000 0001 	and.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c4 ff8c 0000 0200 	and.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c4 f0aa 0000 0200 	and.f.ge	0,0x200,0x2
