	.globl	f3
	.ent	f3
f3:
	lw	$4,%got_page(g + 0x8000)($gp)
	addiu	$4,$4,%got_ofst(g + 0x8000)
	.end	f3
