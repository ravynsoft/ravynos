#as: -mcpu=arcem
#objdump: -dr -M quarkse_em

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	392a 0080           	dsp_fp_div	r0,r1,r2
   4:	3b2a 371a           	dsp_fp_div	gp,fp,sp
   8:	3d2a 37dd           	dsp_fp_div	ilink,ilink,blink
   c:	396a 0000           	dsp_fp_div	r0,r1,0
  10:	3e2a 7080 0000 0000 	dsp_fp_div	r0,0,r2
  18:	392a 00be           	dsp_fp_div	0,r1,r2
  1c:	392a 0f80 ffff ffff 	dsp_fp_div	r0,r1,0xffffffff
  24:	3e2a 7080 ffff ffff 	dsp_fp_div	r0,0xffffffff,r2
  2c:	392a 0f80 0000 00ff 	dsp_fp_div	r0,r1,0xff
  34:	3e2a 7080 0000 00ff 	dsp_fp_div	r0,0xff,r2
  3c:	392a 0f80 ffff ff00 	dsp_fp_div	r0,r1,0xffffff00
  44:	3e2a 7080 ffff ff00 	dsp_fp_div	r0,0xffffff00,r2
  4c:	39aa 0004           	dsp_fp_div	r1,r1,256
  50:	396a 0fc0           	dsp_fp_div	r0,r1,0x3f
  54:	3e2a 7080 ffff feff 	dsp_fp_div	r0,0xfffffeff,r2
  5c:	3e2a 7f80 0000 0100 	dsp_fp_div	r0,0x100,0x100
  64:	392a 0f80 0000 0000 	dsp_fp_div	r0,r1,0
			68: R_ARC_32_ME	foo
  6c:	38ea 0080           	dsp_fp_div	r0,r0,r2
  70:	3bea 0140           	dsp_fp_div	r3,r3,r5
  74:	3eea 0201           	dsp_fp_div.eq	r6,r6,r8
  78:	39ea 12c1           	dsp_fp_div.eq	r9,r9,r11
  7c:	3cea 1382           	dsp_fp_div.ne	r12,r12,r14
  80:	3fea 1442           	dsp_fp_div.ne	r15,r15,r17
  84:	3aea 2503           	dsp_fp_div.p	r18,r18,r20
  88:	3dea 25c3           	dsp_fp_div.p	r21,r21,r23
  8c:	38ea 3684           	dsp_fp_div.n	r24,r24,gp
  90:	3bea 3744           	dsp_fp_div.n	fp,fp,ilink
  94:	3eea 37c5           	dsp_fp_div.c	r30,r30,blink
  98:	3bea 00c5           	dsp_fp_div.c	r3,r3,r3
  9c:	3bea 0205           	dsp_fp_div.c	r3,r3,r8
  a0:	3bea 0106           	dsp_fp_div.nc	r3,r3,r4
  a4:	3cea 0106           	dsp_fp_div.nc	r4,r4,r4
  a8:	3cea 01c6           	dsp_fp_div.nc	r4,r4,r7
  ac:	3cea 0147           	dsp_fp_div.v	r4,r4,r5
  b0:	3dea 0147           	dsp_fp_div.v	r5,r5,r5
  b4:	3dea 0148           	dsp_fp_div.nv	r5,r5,r5
  b8:	3dea 0148           	dsp_fp_div.nv	r5,r5,r5
  bc:	3eea 0009           	dsp_fp_div.gt	r6,r6,r0
  c0:	38ea 002a           	dsp_fp_div.ge	r0,r0,0
  c4:	39ea 006b           	dsp_fp_div.lt	r1,r1,0x1
  c8:	3bea 00ed           	dsp_fp_div.hi	r3,r3,0x3
  cc:	3cea 012e           	dsp_fp_div.ls	r4,r4,0x4
  d0:	3dea 016f           	dsp_fp_div.pnz	r5,r5,0x5
  d4:	392a 8080           	dsp_fp_div.f	r0,r1,r2
  d8:	396a 8040           	dsp_fp_div.f	r0,r1,0x1
  dc:	3e2a f080 0000 0001 	dsp_fp_div.f	r0,0x1,r2
  e4:	392a 80be           	dsp_fp_div.f	0,r1,r2
  e8:	392a 8f80 0000 0200 	dsp_fp_div.f	r0,r1,0x200
  f0:	3e2a f080 0000 0200 	dsp_fp_div.f	r0,0x200,r2
  f8:	39ea 8081           	dsp_fp_div.eq.f	r1,r1,r2
  fc:	38ea 8022           	dsp_fp_div.ne.f	r0,r0,0
 100:	3aea 808b           	dsp_fp_div.lt.f	r2,r2,r2
 104:	3eea f0a9 0000 0001 	dsp_fp_div.gt.f	0,0x1,0x2
 10c:	3eea ff8c 0000 0200 	dsp_fp_div.le.f	0,0x200,0x200
 114:	3eea f0aa 0000 0200 	dsp_fp_div.ge.f	0,0x200,0x2
