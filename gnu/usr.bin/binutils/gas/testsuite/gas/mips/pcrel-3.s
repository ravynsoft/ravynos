	.text
	.ent	func
func:
	lui	$4,%hi(foo-.)
	addiu	$4,%lo(foo-.)
	lw	$4,%got(foo-.)($gp)
	.end	func

	.byte	foo-.
	.half	foo-.
	.quad	foo-.
