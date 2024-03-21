#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 202f 0047           	extb	r0,r1
0x[0-9a-f]+ 232f 3707           	extb	fp,sp
0x[0-9a-f]+ 206f 0007           	extb	r0,0
0x[0-9a-f]+ 212f 0f87 ffff ffff 	extb	r1,0xffffffff
0x[0-9a-f]+ 262f 7087           	extb	0,r2
0x[0-9a-f]+ 242f 0f87 0000 00ff 	extb	r4,0xff
0x[0-9a-f]+ 262f 0f87 ffff ff00 	extb	r6,0xffffff00
0x[0-9a-f]+ 202f 1f87 0000 0100 	extb	r8,0x100
0x[0-9a-f]+ 212f 1f87 ffff feff 	extb	r9,0xfffffeff
0x[0-9a-f]+ 232f 1f87 4242 4242 	extb	r11,0x42424242
0x[0-9a-f]+ 202f 0f87 0000 0000 	extb	r0,0
			44: R_ARC_32_ME	foo
0x[0-9a-f]+ 202f 8047           	extb.f	r0,r1
0x[0-9a-f]+ 226f 8047           	extb.f	r2,0x1
0x[0-9a-f]+ 262f f107           	extb.f	0,r4
0x[0-9a-f]+ 252f 8f87 0000 0200 	extb.f	r5,0x200
