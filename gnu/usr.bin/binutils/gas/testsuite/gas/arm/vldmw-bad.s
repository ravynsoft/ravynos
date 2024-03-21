	.syntax unified

foo:
	vldmia 	pc!, {d0-d1}
	b	bar
baz:
	.word   0x00000000, 0x3ff00000, 0x9999999a, 0x3ff19999
bar:
	vstmia pc!, {d0-d1}
	b	foo2
baz2:
	.word   0x00000000, 0x3ff00000, 0x9999999a, 0x3ff19999
foo2:
	nop
	nop

