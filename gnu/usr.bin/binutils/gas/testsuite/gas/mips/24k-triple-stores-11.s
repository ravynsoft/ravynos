	.text
foo:
	add   $4,$4,$4
	add   $4,$4,$4
	add   $4,$4,$4
	add   $4,$4,$4
	sw    $2,%gp_rel(sym1)($28)
	sw    $3,%gp_rel(sym2)($28)
	sw    $4,%gp_rel(sym3)($28)
