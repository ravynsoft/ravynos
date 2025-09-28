#as: -mcpu=archs
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 1200 3180           	ldd	r0r1,\[gp\]
			0: R_ARC_SDA_LDST	b
0x[0-9a-f]+ 1a00 3086           	std	r2r3,\[gp\]
			4: R_ARC_SDA_LDST	b
0x[0-9a-f]+ 1200 3780           	ldd.as	r0r1,\[gp\]
			8: R_ARC_SDA_LDST2	b
0x[0-9a-f]+ 1a00 309e           	std.as	r2r3,\[gp\]
			c: R_ARC_SDA_LDST2	b
0x[0-9a-f]+ 5000                	ld_s	r1,\[gp,0\]
			10: R_ARC_SDA16_ST2	b
0x[0-9a-f]+ 5010                	st_s	r0,\[gp,0\]
			12: R_ARC_SDA16_ST2	b
