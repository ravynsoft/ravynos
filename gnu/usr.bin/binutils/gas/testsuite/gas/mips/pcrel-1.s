	.text
	.ent	func
func:
	lui	$4,%hi(foo-.)
	addiu	$4,%lo(foo-.)
	.end	func

	.space	0x8008

	.ent	foo
foo:
	nop
	.end	foo
