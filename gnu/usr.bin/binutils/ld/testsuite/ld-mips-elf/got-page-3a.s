	.macro	makeref,sym
	lw	$5,%got(\sym\@)($gp)
	.endm

	.globl	f1
	.ent	f1
f1:
	# See below.
	lw	$4,%got(foo+0x120000)($gp)
	addiu	$4,$4,%lo(foo+0x120000)

	# 2 pages
	lw	$4,%got(foo-0x8000)($gp)
	addiu	$4,$4,%lo(foo-0x8000)
	lw	$4,%got(foo+0x800)($gp)
	addiu	$4,$4,%lo(foo+0x8000)

	# 2 pages
	lw	$4,%got(foo-0x1000000)($gp)
	addiu	$4,$4,%lo(foo-0x1000000)
	lw	$4,%got(foo-0xffffff)($gp)
	addiu	$4,$4,%lo(foo-0xffffff)

	# 1 page
	lw	$4,%got(foo+0x120000)($gp)
	addiu	$4,$4,%lo(foo+0x120000)

	# 5 pages
	lw	$4,%got(bar)($gp)
	addiu	$4,$4,%lo(bar)
	lw	$4,%got(bar+0x20000)($gp)
	addiu	$4,$4,%lo(bar+0x20000)
	lw	$4,%got(bar+0x40000)($gp)
	addiu	$4,$4,%lo(bar+0x40000)
	lw	$4,%got(bar+0x30000)($gp)
	addiu	$4,$4,%lo(bar+0x30000)
	lw	$4,%got(bar+0x10000)($gp)
	addiu	$4,$4,%lo(bar+0x10000)
	lw	$4,%got(bar+0x38000)($gp)
	addiu	$4,$4,%lo(bar+0x38000)
	lw	$4,%got(bar+0x14000)($gp)
	addiu	$4,$4,%lo(bar+0x14000)
	lw	$4,%got(bar+0x2c000)($gp)
	addiu	$4,$4,%lo(bar+0x2c000)
	lw	$4,%got(bar+0x02000)($gp)
	addiu	$4,$4,%lo(bar+0x02000)
	.end	f1

	.rept	8000
	makeref	foo
	.endr

	.section .bss.foo,"aw",@nobits
	.fill	0x800000
foo:	.fill	0x800000

	.section .bss.bar,"aw",@nobits
bar:	.fill	0xc00000
