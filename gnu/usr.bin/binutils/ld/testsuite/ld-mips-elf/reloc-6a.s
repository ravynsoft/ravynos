	.globl	us .text
	.globl	gs .text
us:
gs:
ls:
	lw	$4,%got_page(us)($gp)
	addiu	$4,$4,%got_ofst(us)
	lw	$4,%got_page(gs)($gp)
	addiu	$4,$4,%got_ofst(gs)
	lw	$4,%got_page(ls)($gp)
	addiu	$4,$4,%got_ofst(ls)
