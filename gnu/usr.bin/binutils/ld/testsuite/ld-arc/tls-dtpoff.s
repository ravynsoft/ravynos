	.cpu HS
;;; Test if palette is resolved by the linker.
	add_s r0,r0,@palette@dtpoff+2048

	.section	.tbss,"awT",@nobits
	.align 4
	.zero	4
	.align 4
	.type	palette, @object
	.size	palette, 6144
palette:
	.zero	6144
