#as: -mcpu=archs
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2730 7f82 0000 0000 	ld	r2,\[pcl,0\].*
			4: R_ARC_TLS_IE_GOT	var
0x[0-9a-f]+ 2700 7f80 0000 0000 	add	r0,pcl,0.*
			c: R_ARC_TLS_GD_GOT	var
0x[0-9a-f]+ 2000 0f81 0000 0000 	add	r1,r0,0
			14: R_ARC_TLS_DTPOFF	var
0x[0-9a-f]+ 2100 3f80 0000 0000 	add	r0,r25,0
			1c: R_ARC_TLS_LE_32	var
0x[0-9a-f]+ 0802 0000           	bl	0	;0x00000000
			20: R_ARC_TLS_GD_LD	.tdata
			20: R_ARC_S25W_PCREL_PLT	func
