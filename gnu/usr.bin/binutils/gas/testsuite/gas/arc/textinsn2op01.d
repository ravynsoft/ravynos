#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	3830 007e           	myinsn	r0,r1
   4:	3b30 373e           	myinsn	fp,sp
   8:	3870 003e           	myinsn	r0,0
   c:	3930 0fbe ffff ffff 	myinsn	r1,0xffffffff
  14:	3ef0 7080 0000 0000 	myinsn	0,r2
  1c:	3c30 0fbe 0000 00ff 	myinsn	r4,0xff
  24:	3e30 0fbe ffff ff00 	myinsn	r6,0xffffff00
  2c:	3830 1fbe 0000 0100 	myinsn	r8,0x100
  34:	3930 1fbe ffff feff 	myinsn	r9,0xfffffeff
  3c:	3b30 1fbe 4242 4242 	myinsn	r11,0x42424242
  44:	3830 0fbe 0000 0000 	myinsn	r0,0
			48: R_ARC_32_ME	foo
  4c:	3830 807e           	myinsn.f	r0,r1
  50:	3a70 807e           	myinsn.f	r2,0x1
  54:	3ef0 f100 0000 0000 	myinsn.f	0,r4
  5c:	3d30 8fbe 0000 0200 	myinsn.f	r5,0x200
  64:	3ef0 f102 0000 0000 	myinsn.ne.f	0,r4
  6c:	3ef0 ff85 dead beef 	myinsn.c.f	0xdeadbeef,0xdeadbeef
  74:	3ef0 f0a6 dead beef 	myinsn.nc.f	0xdeadbeef,0x2
