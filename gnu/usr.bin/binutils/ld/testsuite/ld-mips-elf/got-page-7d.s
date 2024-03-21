	.globl	f4
	.ent	f4
f4:
	lw	$4,%got_page(g + 0x10000)($gp)
	addiu	$4,$4,%got_ofst(g + 0x10000)
	.end	f4
