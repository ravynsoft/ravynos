#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 200a 0040           	mov	r0,r1
0x[0-9a-f]+ 230a 3700           	mov	fp,sp
0x[0-9a-f]+ 204a 0000           	mov	r0,0
0x[0-9a-f]+ 218a 0fff           	mov	r1,-1
0x[0-9a-f]+ 260a 7080           	mov	0,r2
0x[0-9a-f]+ 248a 0fc3           	mov	r4,255
0x[0-9a-f]+ 268a 7fc3           	mov	0,255
0x[0-9a-f]+ 268a 003c           	mov	r6,-256
0x[0-9a-f]+ 230a 1f80 4242 4242 	mov	r11,0x42424242
0x[0-9a-f]+ 260a 7f80 1234 5678 	mov	0,0x12345678
0x[0-9a-f]+ 200a 0f80 0000 0000 	mov	r0,0
			34: R_ARC_32_ME	foo
0x[0-9a-f]+ 20ca 0040           	mov	r0,r1
0x[0-9a-f]+ 23ca 0100           	mov	r3,r4
0x[0-9a-f]+ 26ca 01c1           	mov.eq	r6,r7
0x[0-9a-f]+ 21ca 1281           	mov.eq	r9,r10
0x[0-9a-f]+ 24ca 1342           	mov.ne	r12,r13
0x[0-9a-f]+ 27ca 1402           	mov.ne	r15,r16
0x[0-9a-f]+ 22ca 24c3           	mov.p	r18,r19
0x[0-9a-f]+ 25ca 2583           	mov.p	r21,r22
0x[0-9a-f]+ 20ca 3644           	mov.n	r24,r25
0x[0-9a-f]+ 23ca 3704           	mov.n	fp,sp
0x[0-9a-f]+ 20ca 0045           	mov.c	r0,r1
0x[0-9a-f]+ 23ca 0105           	mov.c	r3,r4
0x[0-9a-f]+ 26ca 01c5           	mov.c	r6,r7
0x[0-9a-f]+ 21ca 1006           	mov.nc	r9,r0
0x[0-9a-f]+ 22ca 00c6           	mov.nc	r2,r3
0x[0-9a-f]+ 25ca 0186           	mov.nc	r5,r6
0x[0-9a-f]+ 20ca 1247           	mov.v	r8,r9
0x[0-9a-f]+ 21ca 0087           	mov.v	r1,r2
0x[0-9a-f]+ 24ca 0148           	mov.nv	r4,r5
0x[0-9a-f]+ 27ca 0208           	mov.nv	r7,r8
0x[0-9a-f]+ 20ca 0009           	mov.gt	r0,r0
0x[0-9a-f]+ 20ca 002a           	mov.ge	r0,0
0x[0-9a-f]+ 26ca 704b           	mov.lt	0,r1
0x[0-9a-f]+ 26ca 70ac           	mov.le	0,0x2
0x[0-9a-f]+ 23ca 00cd           	mov.hi	r3,r3
0x[0-9a-f]+ 24ca 010e           	mov.ls	r4,r4
0x[0-9a-f]+ 25ca 014f           	mov.pnz	r5,r5
0x[0-9a-f]+ 200a 8040           	mov.f	r0,r1
0x[0-9a-f]+ 224a 8040           	mov.f	r2,0x1
0x[0-9a-f]+ 260a f100           	mov.f	0,r4
0x[0-9a-f]+ 258a 8008           	mov.f	r5,512
0x[0-9a-f]+ 20ca 8041           	mov.f.eq	r0,r1
0x[0-9a-f]+ 21ca 8022           	mov.f.ne	r1,0
0x[0-9a-f]+ 26ca f08b           	mov.f.lt	0,r2
0x[0-9a-f]+ 26ca f089           	mov.f.gt	0,r2
0x[0-9a-f]+ 20ca 8f8c 0000 0200 	mov.f.le	r0,0x200
0x[0-9a-f]+ 26ca f08a           	mov.f.ge	0,r2
0x[0-9a-f]+ 26ca ff84 0000 0200 	mov.f.n	0,0x200
