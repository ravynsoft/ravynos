#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ ce00                	add_s	r0,gp,0
			0: R_ARC_SDA16_LD2	a
0x[0-9a-f]+ c800                	ld_s	r0,\[gp,0\]
			2: R_ARC_SDA16_LD2	a
0x[0-9a-f]+ cc00                	ldw_s	r0,\[gp,0\]
			4: R_ARC_SDA16_LD1	a
0x[0-9a-f]+ ca00                	ldb_s	r0,\[gp,0\]
			6: R_ARC_SDA16_LD	a
0x[0-9a-f]+ 1200 360c           	ld.as	r12,\[gp\]
			8: R_ARC_SDA_LDST2	a
0x[0-9a-f]+ 1a00 3398           	st.as	r14,\[gp\]
			c: R_ARC_SDA_LDST2	a
0x[0-9a-f]+ 1200 300a           	ld	r10,\[gp\]
			10: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1a00 3240           	st	r9,\[gp\]
			14: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1200 3108           	ldh	r8,\[gp\]
			18: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1a00 31c4           	sth	r7,\[gp\]
			1c: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1200 3086           	ldb	r6,\[gp\]
			20: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1a00 3142           	stb	r5,\[gp\]
			24: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1200 3708           	ldh.as	r8,\[gp\]
			28: R_ARC_SDA_LDST1	a
0x[0-9a-f]+ 1a00 31dc           	sth.as	r7,\[gp\]
			2c: R_ARC_SDA_LDST1	a
0x[0-9a-f]+ 1200 3688           	ldb.as	r8,\[gp\]
			30: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 1a00 31da           	stb.as	r7,\[gp\]
			34: R_ARC_SDA_LDST	a
0x[0-9a-f]+ 2200 3f81 0000 0000 	add	r1,gp,0
			3c: R_ARC_SDA32_ME	a
