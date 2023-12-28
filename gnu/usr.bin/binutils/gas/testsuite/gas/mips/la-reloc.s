	.ent	func
func:
	la	$4,%lo(foo)
	la	$4,%hi(foo)
	la	$4,%lo(0x12348765)
	la	$4,%hi(0x12348765)
	la	$4,%lo(foo)($5)
	la	$4,%hi(foo)($5)
	la	$4,%lo(0x12348765)($5)
	la	$4,%hi(0x12348765)($5)
	la	$4,%lo(foo+0x12348765)($5)
	la	$4,%hi(foo+0x12348765)($5)
	.end	func
