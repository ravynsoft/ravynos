	.globl	foo
	.ent	foo
foo:
	# Create page references to 16 values.  The layout of the values
	# in this input file requires at most 2 page entries.
	.set	y,0x02000000
	.rept	16
	lw	$4,%got_page(1f)($gp)
	addiu	$4,$4,%got_ofst(1f)
	.section .rodata.cst4,"aM",@progbits,4
1:	.word	y
	.set	y,y+1
	.text
	.endr
	.end	foo

	# Make sure the loadable size of the library is large.
	.section .bss
	.globl	g
g:
	.space	0x800000
