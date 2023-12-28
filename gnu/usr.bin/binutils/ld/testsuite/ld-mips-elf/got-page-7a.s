	.globl	f1
	.ent	f1
f1:
	lw	$4,%got_page(g)($gp)
	addiu	$4,$4,%got_ofst(g)
	.end	f1
