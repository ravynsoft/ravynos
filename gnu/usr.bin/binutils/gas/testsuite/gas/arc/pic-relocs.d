#as: -mcpu=archs
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2730 7f82 0000 0000 	ld	r2,\[pcl,0\].*
			4: R_ARC_GOTPC32	var
0x[0-9a-f]+ 2700 7f9a 0000 0000 	add	gp,pcl,0.*
			c: R_ARC_GOTPC32	var
0x[0-9a-f]+ 2200 3f82 0000 0000 	add	r2,gp,0
			14: R_ARC_GOTOFF	var
