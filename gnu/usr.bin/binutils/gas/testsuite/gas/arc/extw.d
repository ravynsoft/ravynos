#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x00000000 202f 0048           	ext[hw]+	r0,r1
0x00000004 232f 3708           	ext[hw]+	fp,sp
0x00000008 206f 0008           	ext[hw]+	r0,0
0x0000000c 212f 0f88 ffff ffff 	ext[hw]+	r1,0xffffffff
0x00000014 262f 7088           	ext[hw]+	0,r2
0x00000018 242f 0f88 0000 00ff 	ext[hw]+	r4,0xff
0x00000020 262f 0f88 ffff ff00 	ext[hw]+	r6,0xffffff00
0x00000028 202f 1f88 0000 0100 	ext[hw]+	r8,0x100
0x00000030 212f 1f88 ffff feff 	ext[hw]+	r9,0xfffffeff
0x00000038 232f 1f88 4242 4242 	ext[hw]+	r11,0x42424242
0x00000040 202f 0f88 0000 0000 	ext[hw]+	r0,0
			44: R_ARC_32_ME	foo
0x00000048 202f 8048           	ext[hw]+.f	r0,r1
0x0000004c 226f 8048           	ext[hw]+.f	r2,0x1
0x00000050 262f f108           	ext[hw]+.f	0,r4
0x00000054 252f 8f88 0000 0200 	ext[hw]+.f	r5,0x200
