	.h8300s
	.global var1,var2,var3,var4,var5,var6

	.equ	var1,0xffffff00
	.equ	var2,0xfffffff0
	.equ	var3,0xffff8000

	.equ	var4,0xffff7ff0
	.equ	var5,0xffff7ff8
	.equ	var6,0x00ffff00

	.section	.rodata.tab2,"a",@progbits
	.align 2
table2:		# no relax in sections other than text expected:
	.short	0x0100
			# MOV.L @(d:24,ERs),ERd opcodes
	.short	0x7800
	.short	0x6b20
	.long	table2
