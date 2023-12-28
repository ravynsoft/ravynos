	.text
	.ent	foo
	.option	pic0
foo:
	la	$2, bar
	jr	$31
	.option	pic2
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
