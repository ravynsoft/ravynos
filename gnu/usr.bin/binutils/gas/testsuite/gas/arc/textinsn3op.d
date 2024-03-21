#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	3930 0080           	myinsn	r0,r1,r2
   4:	3b30 371a           	myinsn	gp,fp,sp
   8:	3e30 37dd           	myinsn	ilink,r30,blink
   c:	3970 0000           	myinsn	r0,r1,0
  10:	3e30 7080 0000 0000 	myinsn	r0,0,r2
  18:	3930 00be           	myinsn	0,r1,r2
  1c:	3930 0f80 ffff ffff 	myinsn	r0,r1,0xffffffff
  24:	3e30 7080 ffff ffff 	myinsn	r0,0xffffffff,r2
  2c:	3930 0f80 0000 00ff 	myinsn	r0,r1,0xff
  34:	3e30 7080 0000 00ff 	myinsn	r0,0xff,r2
  3c:	3930 0f80 ffff ff00 	myinsn	r0,r1,0xffffff00
  44:	3e30 7080 ffff ff00 	myinsn	r0,0xffffff00,r2
  4c:	3930 0f80 0000 0100 	myinsn	r0,r1,0x100
  54:	3e30 7080 ffff feff 	myinsn	r0,0xfffffeff,r2
  5c:	3e30 7f80 0000 0100 	myinsn	r0,0x100,0x100
  64:	3930 0f80 0000 0000 	myinsn	r0,r1,0
			68: R_ARC_32_ME	foo
  6c:	38f0 0080           	myinsn	r0,r0,r2
  70:	3bf0 0140           	myinsn	r3,r3,r5
  74:	3ef0 0201           	myinsn.eq	r6,r6,r8
  78:	39f0 12c1           	myinsn.eq	r9,r9,r11
  7c:	3cf0 1382           	myinsn.ne	r12,r12,r14
  80:	3ff0 1442           	myinsn.ne	r15,r15,r17
  84:	3af0 2503           	myinsn.p	r18,r18,r20
  88:	3df0 25c3           	myinsn.p	r21,r21,r23
  8c:	38f0 3684           	myinsn.n	r24,r24,gp
  90:	3bf0 3744           	myinsn.n	fp,fp,ilink
  94:	3ef0 37c5           	myinsn.c	r30,r30,blink
  98:	3bf0 00c5           	myinsn.c	r3,r3,r3
  9c:	3bf0 0205           	myinsn.c	r3,r3,r8
  a0:	3bf0 0106           	myinsn.nc	r3,r3,r4
  a4:	3cf0 0106           	myinsn.nc	r4,r4,r4
  a8:	3cf0 01c6           	myinsn.nc	r4,r4,r7
  ac:	3cf0 0147           	myinsn.v	r4,r4,r5
  b0:	3df0 0147           	myinsn.v	r5,r5,r5
  b4:	3df0 0148           	myinsn.nv	r5,r5,r5
  b8:	3df0 0148           	myinsn.nv	r5,r5,r5
  bc:	3ef0 0009           	myinsn.gt	r6,r6,r0
  c0:	38f0 002a           	myinsn.ge	r0,r0,0
  c4:	39f0 006b           	myinsn.lt	r1,r1,0x1
  c8:	3bf0 00ed           	myinsn.hi	r3,r3,0x3
  cc:	3cf0 012e           	myinsn.ls	r4,r4,0x4
  d0:	3df0 016f           	myinsn.pnz	r5,r5,0x5
  d4:	3930 8080           	myinsn.f	r0,r1,r2
  d8:	3970 8040           	myinsn.f	r0,r1,0x1
  dc:	3e30 f080 0000 0001 	myinsn.f	r0,0x1,r2
  e4:	3930 80be           	myinsn.f	0,r1,r2
  e8:	3930 8f80 0000 0200 	myinsn.f	r0,r1,0x200
  f0:	3e30 f080 0000 0200 	myinsn.f	r0,0x200,r2
  f8:	39f0 8081           	myinsn.eq.f	r1,r1,r2
  fc:	38f0 8022           	myinsn.ne.f	r0,r0,0
 100:	3af0 808b           	myinsn.lt.f	r2,r2,r2
 104:	3ef0 f0a9 0000 0001 	myinsn.gt.f	0,0x1,0x2
 10c:	3ef0 ff8c 0000 0200 	myinsn.le.f	0,0x200,0x200
 114:	3ef0 f0aa 0000 0200 	myinsn.ge.f	0,0x200,0x2
