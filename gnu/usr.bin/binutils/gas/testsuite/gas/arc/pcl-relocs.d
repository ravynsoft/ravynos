#as: -mcpu=archs
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 2700 7f80 0000 0000 	add	r0,pcl,0.*
			4: R_ARC_PC32	var
0x[0-9a-f]+ 2736 7f86 0000 0000 	ldd	r6r7,\[pcl,0\].*
			c: R_ARC_PC32	var
0x[0-9a-f]+ 2730 7f83 0000 0000 	ld	r3,\[pcl,0\].*
			14: R_ARC_PC32	var
