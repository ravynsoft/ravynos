	.section .data,"awG",%progbits,foo_group,comdat
	.dc.a	x_alias
	.type	x, %object
	.p2align 2
	.size	x, 4
x:
	.zero	4
	.globl	x_alias
	.set	x_alias,x
