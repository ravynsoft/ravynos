	.globl	f2
	.ent	f2
f2:
	lw	$4,%got_page(g + 0x4000)($gp)
	addiu	$4,$4,%got_ofst(g + 0x4000)
	.end	f2
