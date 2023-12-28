	.ent	foo
foo:
	addiu	$4,.-foo
	nop
.L1:
	nop
	addiu	$4,.L2 - .
	addiu	$4,. - .L1
	addiu	$4,. - foo
.L2:
	addiu	$4,%lo(.L2 - .)
	addiu	$4,%lo(.L3 - .)
	addiu	$4,%lo(. - .L1)
	addiu	$4,%lo(. - foo)
	nop
.L3:
	nop
	.end	foo
