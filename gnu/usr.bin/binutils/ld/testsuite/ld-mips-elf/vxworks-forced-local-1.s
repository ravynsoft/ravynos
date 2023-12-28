	.globl	foo1
	.globl	foo2
	.globl	foo3
	lw	$4,%call16(foo1)($gp)
	lw	$4,%call16(foo2)($gp)
	lw	$4,%call16(foo3)($gp)
	lw	$4,%got(bar)($gp)
foo1:
	nop
foo2:
	nop
foo3:
	nop
