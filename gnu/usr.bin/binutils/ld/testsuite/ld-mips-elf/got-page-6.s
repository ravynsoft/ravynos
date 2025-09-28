	# Create a mergeable section full of a single value.
	# Create page references relative to instances of the value
	# that are large distances apart.
	#
	# The mergeable entries collapse to one, so even with the
	# large distances in the original file, we should end
	# up with a single page entry.
	.globl	foo
	.ent	foo
foo:
	.rept	4
	lw	$4,%got_page(1f)($gp)
	addiu	$4,$4,%got_ofst(1f)
	.section .rodata.cst4,"aM",@progbits,4
1:
	.rept	0x8000
	.word	123456
	.endr
	.text
	.endr
	.end	foo

	# Make sure the loadable size of the library is large.
	.section .bss
	.globl	g
g:
	.space	0x800000
