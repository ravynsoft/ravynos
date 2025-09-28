	.ent	func
func:
	ld	$4,($4)
	ld	$4,0x7ffb($4)
	ld	$4,0x7ffc($4)
	ld	$4,0x7fff($4)
	ld	$4,0x8000($4)

	ld	$4,($5)
	ld	$4,0x7ffb($5)
	ld	$4,0x7ffc($5)
	ld	$4,0x7fff($5)
	ld	$4,0x8000($5)
	ld	$4,0x37ffb($5)
	ld	$4,0x37ffc($5)
	ld	$4,0x37fff($5)
	ld	$4,0x38000($5)

	ld	$4,%lo(foo)
	ld	$4,%hi(foo)
	ld	$4,%gp_rel(foo)
	ld	$4,%lo(0x12348765)
	ld	$4,%hi(0x12348765)

	ld	$4,%lo(foo)($4)
	ld	$4,%hi(foo)($4)
	ld	$4,%gp_rel(foo)($4)

	ld	$4,%lo(foo)($5)
	ld	$4,%hi(foo)($5)
	ld	$4,%gp_rel(foo)($5)
	ld	$4,%lo(0x12348765)($5)
	ld	$4,%hi(0x12348765)($5)
	ld	$4,%lo(foo+0x12348765)($5)
	ld	$4,%hi(foo+0x12348765)($5)
	.end	func
