	.section	.init_array
	.align 4
	.type	init_array, %object
	.size	init_array, 4
init_array:
	.dc.a	foo
	.section	.preinit_array
	.align 4
	.type	preinit_array, %object
	.size	preinit_array, 4
preinit_array:
	.dc.a	foo
	.section	.fini_array
	.align 4
	.type	fini_array, %object
	.size	fini_array, 4
fini_array:
	.dc.a	foo
