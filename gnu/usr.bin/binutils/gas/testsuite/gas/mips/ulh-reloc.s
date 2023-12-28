	.ent	func
func:
	ulh	$4,($4)
	ulh	$4,0x7ffe($4)
	ulh	$4,0x7fff($4)
	ulh	$4,0x8000($4)

	ulh	$4,($5)
	ulh	$4,0x7ffe($5)
	ulh	$4,0x7fff($5)
	ulh	$4,0x8000($5)
	ulh	$4,0x37ffe($5)
	ulh	$4,0x37fff($5)
	ulh	$4,0x38000($5)

	ulh	$4,%lo(foo)
	ulh	$4,%hi(foo)
	ulh	$4,%gp_rel(foo)
	ulh	$4,%lo(0x12348765)
	ulh	$4,%hi(0x12348765)

	ulh	$4,%lo(foo)($4)
	ulh	$4,%hi(foo)($4)
	ulh	$4,%gp_rel(foo)($4)

	ulh	$4,%lo(foo)($5)
	ulh	$4,%hi(foo)($5)
	ulh	$4,%gp_rel(foo)($5)
	ulh	$4,%lo(0x12348765)($5)
	ulh	$4,%hi(0x12348765)($5)
	ulh	$4,%lo(foo+0x12348765)($5)
	ulh	$4,%hi(foo+0x12348765)($5)
	.end	func
