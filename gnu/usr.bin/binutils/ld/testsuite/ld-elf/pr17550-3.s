	.data
	.dc.a	y
	.section .data,"awG",%progbits,foo_group,comdat
	.type	y, %object
	.size	y, 4
y:
	.zero	4
	.globl	x
	.type	x, %object
	.size	x, 4
x:
	.zero	4
	.globl	x_alias
	.set	x_alias,x
