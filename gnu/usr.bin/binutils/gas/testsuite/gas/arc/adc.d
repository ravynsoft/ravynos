#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2101 0080           	adc	r0,r1,r2
0x[0-9a-f]+ 2301 371a           	adc	gp,fp,sp
0x[0-9a-f]+ 2601 37dd           	adc	ilink,r30,blink
0x[0-9a-f]+ 2141 0000           	adc	r0,r1,0
0x[0-9a-f]+ 2601 7080 0000 0000 	adc	r0,0,r2
0x[0-9a-f]+ 2101 00be           	adc	0,r1,r2
0x[0-9a-f]+ 2101 0f80 ffff ffff 	adc	r0,r1,0xffffffff
0x[0-9a-f]+ 2601 7080 ffff ffff 	adc	r0,0xffffffff,r2
0x[0-9a-f]+ 2101 0f80 0000 00ff 	adc	r0,r1,0xff
0x[0-9a-f]+ 2601 7080 0000 00ff 	adc	r0,0xff,r2
0x[0-9a-f]+ 2101 0f80 ffff ff00 	adc	r0,r1,0xffffff00
0x[0-9a-f]+ 2601 7080 ffff ff00 	adc	r0,0xffffff00,r2
0x[0-9a-f]+ 2101 0f80 0000 0100 	adc	r0,r1,0x100
0x[0-9a-f]+ 2601 7080 ffff feff 	adc	r0,0xfffffeff,r2
0x[0-9a-f]+ 2601 7f80 0000 0100 	adc	r0,0x100,0x100
0x[0-9a-f]+ 2101 0f80 0000 0000 	adc	r0,r1,0
			68: R_ARC_32_ME	foo
0x[0-9a-f]+ 20c1 0080           	adc	r0,r0,r2
0x[0-9a-f]+ 23c1 0140           	adc	r3,r3,r5
0x[0-9a-f]+ 26c1 0201           	adc.eq	r6,r6,r8
0x[0-9a-f]+ 21c1 12c1           	adc.eq	r9,r9,r11
0x[0-9a-f]+ 24c1 1382           	adc.ne	r12,r12,r14
0x[0-9a-f]+ 27c1 1442           	adc.ne	r15,r15,r17
0x[0-9a-f]+ 22c1 2503           	adc.p	r18,r18,r20
0x[0-9a-f]+ 25c1 25c3           	adc.p	r21,r21,r23
0x[0-9a-f]+ 20c1 3684           	adc.n	r24,r24,gp
0x[0-9a-f]+ 23c1 3744           	adc.n	fp,fp,ilink
0x[0-9a-f]+ 26c1 37c5           	adc.c	r30,r30,blink
0x[0-9a-f]+ 23c1 00c5           	adc.c	r3,r3,r3
0x[0-9a-f]+ 23c1 0205           	adc.c	r3,r3,r8
0x[0-9a-f]+ 23c1 0106           	adc.nc	r3,r3,r4
0x[0-9a-f]+ 24c1 0106           	adc.nc	r4,r4,r4
0x[0-9a-f]+ 24c1 01c6           	adc.nc	r4,r4,r7
0x[0-9a-f]+ 24c1 0147           	adc.v	r4,r4,r5
0x[0-9a-f]+ 25c1 0147           	adc.v	r5,r5,r5
0x[0-9a-f]+ 25c1 0148           	adc.nv	r5,r5,r5
0x[0-9a-f]+ 25c1 0148           	adc.nv	r5,r5,r5
0x[0-9a-f]+ 26c1 0009           	adc.gt	r6,r6,r0
0x[0-9a-f]+ 20c1 002a           	adc.ge	r0,r0,0
0x[0-9a-f]+ 21c1 006b           	adc.lt	r1,r1,0x1
0x[0-9a-f]+ 23c1 00ed           	adc.hi	r3,r3,0x3
0x[0-9a-f]+ 24c1 012e           	adc.ls	r4,r4,0x4
0x[0-9a-f]+ 25c1 016f           	adc.pnz	r5,r5,0x5
0x[0-9a-f]+ 2101 8080           	adc.f	r0,r1,r2
0x[0-9a-f]+ 2141 8040           	adc.f	r0,r1,0x1
0x[0-9a-f]+ 2601 f080 0000 0001 	adc.f	r0,0x1,r2
0x[0-9a-f]+ 2101 80be           	adc.f	0,r1,r2
0x[0-9a-f]+ 2101 8f80 0000 0200 	adc.f	r0,r1,0x200
0x[0-9a-f]+ 2601 f080 0000 0200 	adc.f	r0,0x200,r2
0x[0-9a-f]+ 21c1 8081           	adc.f.eq	r1,r1,r2
0x[0-9a-f]+ 20c1 8022           	adc.f.ne	r0,r0,0
0x[0-9a-f]+ 22c1 808b           	adc.f.lt	r2,r2,r2
0x[0-9a-f]+ 26c1 f0a9 0000 0001 	adc.f.gt	0,0x1,0x2
0x[0-9a-f]+ 26c1 ff8c 0000 0200 	adc.f.le	0,0x200,0x200
0x[0-9a-f]+ 26c1 f0aa 0000 0200 	adc.f.ge	0,0x200,0x2
