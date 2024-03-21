#as: -mcpu=archs
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 0000 0002           	bne	0	;0x00000000
			0: R_ARC_S21H_PCREL_PLT	printf
0x[0-9a-f]+ 0800 0002           	blne	0	;0x00000000
			4: R_ARC_S21W_PCREL_PLT	printf
0x[0-9a-f]+ 0001 0000           	b	0	;0x00000000
			8: R_ARC_S25H_PCREL_PLT	printf
0x[0-9a-f]+ 0802 0000           	bl	0	;0x00000000
			c: R_ARC_S25W_PCREL_PLT	printf
0x[0-9a-f]+ 2700 7f80 0000 0000 	add	r0,pcl,0.*
			14: R_ARC_PLT32	printf
