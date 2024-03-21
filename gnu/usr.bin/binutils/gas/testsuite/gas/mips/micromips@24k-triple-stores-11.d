#objdump: -drz
#as: -mfix-24k -32
#name: 24K: Triple Store (gprel relocs)
#source: 24k-triple-stores-11.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	0084 2110 	add	a0,a0,a0
 *[0-9a-f]+:	0084 2110 	add	a0,a0,a0
 *[0-9a-f]+:	0084 2110 	add	a0,a0,a0
 *[0-9a-f]+:	0084 2110 	add	a0,a0,a0
 *[0-9a-f]+:	f85c 0000 	sw	v0,0\(gp\)
			[0-9a-f]+: R_MICROMIPS_GPREL16	sym1
 *[0-9a-f]+:	f87c 0000 	sw	v1,0\(gp\)
			[0-9a-f]+: R_MICROMIPS_GPREL16	sym2
 *[0-9a-f]+:	f89c 0000 	sw	a0,0\(gp\)
			[0-9a-f]+: R_MICROMIPS_GPREL16	sym3
#pass
