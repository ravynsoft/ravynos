	.ent	func
func:
	dla	$4,%lo(foo)
	dla	$4,%hi(foo)
	dla	$4,%lo(0x12348765)
	dla	$4,%hi(0x12348765)
	dla	$4,%lo(foo)($5)
	dla	$4,%hi(foo)($5)
	dla	$4,%lo(0x12348765)($5)
	dla	$4,%hi(0x12348765)($5)
	dla	$4,%lo(foo+0x12348765)($5)
	dla	$4,%hi(foo+0x12348765)($5)
	dla	$4,%hi(%neg(%gp_rel(bar)))($5)
	dla	$4,%lo(%neg(%gp_rel(bar)))($5)
	.end	func
