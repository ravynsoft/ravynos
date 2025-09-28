#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 202f 0044           	rrc	r0,r1
0x[0-9a-f]+ 232f 3704           	rrc	fp,sp
0x[0-9a-f]+ 206f 0004           	rrc	r0,0
0x[0-9a-f]+ 212f 0f84 ffff ffff 	rrc	r1,0xffffffff
0x[0-9a-f]+ 262f 7084           	rrc	0,r2
0x[0-9a-f]+ 242f 0f84 0000 00ff 	rrc	r4,0xff
0x[0-9a-f]+ 262f 0f84 ffff ff00 	rrc	r6,0xffffff00
0x[0-9a-f]+ 202f 1f84 0000 0100 	rrc	r8,0x100
0x[0-9a-f]+ 212f 1f84 ffff feff 	rrc	r9,0xfffffeff
0x[0-9a-f]+ 232f 1f84 4242 4242 	rrc	r11,0x42424242
0x[0-9a-f]+ 202f 0f84 0000 0000 	rrc	r0,0
			44: R_ARC_32_ME	foo
0x[0-9a-f]+ 202f 8044           	rrc.f	r0,r1
0x[0-9a-f]+ 226f 8044           	rrc.f	r2,0x1
0x[0-9a-f]+ 262f f104           	rrc.f	0,r4
0x[0-9a-f]+ 252f 8f84 0000 0200 	rrc.f	r5,0x200
