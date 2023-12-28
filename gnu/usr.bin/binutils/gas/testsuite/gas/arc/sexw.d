#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 202f 0046           	sex[wh]+	r0,r1
0x[0-9a-f]+ 232f 3706           	sex[wh]+	fp,sp
0x[0-9a-f]+ 206f 0006           	sex[wh]+	r0,0
0x[0-9a-f]+ 212f 0f86 ffff ffff 	sex[wh]+	r1,0xffffffff
0x[0-9a-f]+ 262f 7086           	sex[wh]+	0,r2
0x[0-9a-f]+ 242f 0f86 0000 00ff 	sex[wh]+	r4,0xff
0x[0-9a-f]+ 262f 0f86 ffff ff00 	sex[wh]+	r6,0xffffff00
0x[0-9a-f]+ 202f 1f86 0000 0100 	sex[wh]+	r8,0x100
0x[0-9a-f]+ 212f 1f86 ffff feff 	sex[wh]+	r9,0xfffffeff
0x[0-9a-f]+ 232f 1f86 4242 4242 	sex[wh]+	r11,0x42424242
0x[0-9a-f]+ 202f 0f86 0000 0000 	sex[wh]+	r0,0
			44: R_ARC_32_ME	foo
0x[0-9a-f]+ 202f 8046           	sex[wh]+.f	r0,r1
0x[0-9a-f]+ 226f 8046           	sex[wh]+.f	r2,0x1
0x[0-9a-f]+ 262f f106           	sex[wh]+.f	0,r4
0x[0-9a-f]+ 252f 8f86 0000 0200 	sex[wh]+.f	r5,0x200
