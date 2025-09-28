	# Create a mergeable section full of a single value,
	# and page references relative to one entry called "data".
	#
	# The mergeable entries collapse to one, but the offsets
	# from "data" must still be retained, and need 3 page entries.
	#
	# Technically this isn't valid, because it creates out-of-section
	# page references.  It's still a useful way of making sure that
	# offsets in mergeable sections are handled correctly.
	.globl	foo
	.ent	foo
foo:
	.set	y,0
	.rept	4
	lw	$4,%got_page(data + y)($gp)
	addiu	$4,$4,%got_ofst(data + y)
	.set	y,y+0x8000
	.endr
	.end	foo

	.section .rodata.cst4,"aM",@progbits,4
data:
	.rept	0x8000*4
	.word	123456
	.endr

	# Make sure the loadable size of the library is large.
	.section .bss
	.globl	g
g:
	.space	0x800000
