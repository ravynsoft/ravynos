	.globl	data
	.data
	.p2align 5
	.type	data, %object
	.size	data, 120
data:
	.long	1
	.zero	116
	.globl	foo
	.section	.tbss,"awT",%nobits
	.p2align 2
	.type	foo, %object
	.size	foo, 4
foo:
	.zero	4
	.globl	bar
	.section	.tdata,"awT",%progbits
	.p2align 4
	.type	bar, %object
	.size	bar, 80
bar:
	.long	1
	.zero	76
