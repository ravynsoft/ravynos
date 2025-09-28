	.text
	.globl	foo
	.type foo, %gnu_indirect_function
foo:
	ret
	.size	foo, .-foo
	.pushsection .gnu.build.attributes, "", %note
	.dc.l 6		# size of name
	.dc.l 16	# descsz = sizeof (addresses)
	.dc.l 0x101	# type = FUNC
	.dc.b 0x47, 0x41, 0x2a, 0x2, 0, 0 	# name (numeric: -fstack-protector status)
	.dc.b 0, 0 	# Padding
	.dc.a foo
	.dc.a foo_end	# description (symbol name)
	.popsection

foo_end:
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC0:
	.long	1065353216
