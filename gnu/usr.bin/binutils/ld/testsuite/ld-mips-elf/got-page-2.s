	# See below.
	lw	$4,%got_page(foo+0x120000)($gp)
	addiu	$4,$4,%got_ofst(foo+0x120000)

	# 2 pages
	lw	$4,%got_page(foo-0x8000)($gp)
	addiu	$4,$4,%got_ofst(foo-0x8000)
	lw	$4,%got_page(foo+0x800)($gp)
	addiu	$4,$4,%got_ofst(foo+0x8000)

	# 2 pages
	lw	$4,%got_page(foo-0x1000000)($gp)
	addiu	$4,$4,%got_ofst(foo-0x1000000)
	lw	$4,%got_page(foo-0xffffff)($gp)
	addiu	$4,$4,%got_ofst(foo-0xffffff)

	# 1 page
	lw	$4,%got_page(foo+0x120000)($gp)
	addiu	$4,$4,%got_ofst(foo+0x120000)

	# 5 pages
	lw	$4,%got_page(bar)($gp)
	addiu	$4,$4,%got_ofst(bar)
	lw	$4,%got_page(bar+0x20000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x20000)
	lw	$4,%got_page(bar+0x40000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x40000)
	lw	$4,%got_page(bar+0x30000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x30000)
	lw	$4,%got_page(bar+0x10000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x10000)
	lw	$4,%got_page(bar+0x38000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x38000)
	lw	$4,%got_page(bar+0x14000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x14000)
	lw	$4,%got_page(bar+0x2c000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x2c000)
	lw	$4,%got_page(bar+0x02000)($gp)
	addiu	$4,$4,%got_ofst(bar+0x02000)

	.section .bss.foo,"aw",@nobits
	.fill	0x800000
foo:	.fill	0x800000

	.section .bss.bar,"aw",@nobits
bar:	.fill	0xc00000
