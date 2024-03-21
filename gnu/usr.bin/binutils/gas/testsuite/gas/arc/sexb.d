#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 202f 0045           	sexb	r0,r1
0x[0-9a-f]+ 232f 3705           	sexb	fp,sp
0x[0-9a-f]+ 206f 0005           	sexb	r0,0
0x[0-9a-f]+ 212f 0f85 ffff ffff 	sexb	r1,0xffffffff
0x[0-9a-f]+ 262f 7085           	sexb	0,r2
0x[0-9a-f]+ 242f 0f85 0000 00ff 	sexb	r4,0xff
0x[0-9a-f]+ 262f 0f85 ffff ff00 	sexb	r6,0xffffff00
0x[0-9a-f]+ 202f 1f85 0000 0100 	sexb	r8,0x100
0x[0-9a-f]+ 212f 1f85 ffff feff 	sexb	r9,0xfffffeff
0x[0-9a-f]+ 232f 1f85 4242 4242 	sexb	r11,0x42424242
0x[0-9a-f]+ 202f 0f85 0000 0000 	sexb	r0,0
			44: R_ARC_32_ME	foo
0x[0-9a-f]+ 202f 8045           	sexb.f	r0,r1
0x[0-9a-f]+ 226f 8045           	sexb.f	r2,0x1
0x[0-9a-f]+ 262f f105           	sexb.f	0,r4
0x[0-9a-f]+ 252f 8f85 0000 0200 	sexb.f	r5,0x200
