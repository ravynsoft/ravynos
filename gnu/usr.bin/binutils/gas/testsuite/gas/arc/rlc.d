#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 202f 004b           	rlc	r0,r1
0x[0-9a-f]+ 232f 370b           	rlc	fp,sp
0x[0-9a-f]+ 206f 000b           	rlc	r0,0
0x[0-9a-f]+ 212f 0f8b ffff ffff 	rlc	r1,0xffffffff
0x[0-9a-f]+ 262f 708b           	rlc	0,r2
0x[0-9a-f]+ 242f 0f8b 0000 00ff 	rlc	r4,0xff
0x[0-9a-f]+ 262f 0f8b ffff ff00 	rlc	r6,0xffffff00
0x[0-9a-f]+ 202f 1f8b 0000 0100 	rlc	r8,0x100
0x[0-9a-f]+ 212f 1f8b ffff feff 	rlc	r9,0xfffffeff
0x[0-9a-f]+ 232f 1f8b 4242 4242 	rlc	r11,0x42424242
0x[0-9a-f]+ 202f 0f8b 0000 0000 	rlc	r0,0
			44: R_ARC_32_ME	foo
0x[0-9a-f]+ 202f 804b           	rlc.f	r0,r1
0x[0-9a-f]+ 226f 804b           	rlc.f	r2,0x1
0x[0-9a-f]+ 262f f10b           	rlc.f	0,r4
0x[0-9a-f]+ 252f 8f8b 0000 0200 	rlc.f	r5,0x200
