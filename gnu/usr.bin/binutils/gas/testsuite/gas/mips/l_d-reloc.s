	.ent	func
func:
	l.d	$f4,($5)
	l.d	$f4,0x7ffb($5)
	l.d	$f4,0x7ffc($5)
	l.d	$f4,0x7fff($5)
	l.d	$f4,0x8000($5)
	l.d	$f4,0x37ffb($5)
	l.d	$f4,0x37ffc($5)
	l.d	$f4,0x37fff($5)
	l.d	$f4,0x38000($5)

	l.d	$f4,%lo(foo)
	l.d	$f4,%hi(foo)
	l.d	$f4,%gp_rel(foo)
	l.d	$f4,%lo(0x12348765)
	l.d	$f4,%hi(0x12348765)

	l.d	$f4,%lo(foo)($5)
	l.d	$f4,%hi(foo)($5)
	l.d	$f4,%gp_rel(foo)($5)
	l.d	$f4,%lo(0x12348765)($5)
	l.d	$f4,%hi(0x12348765)($5)
	l.d	$f4,%lo(foo+0x12348765)($5)
	l.d	$f4,%hi(foo+0x12348765)($5)
	.end	func
