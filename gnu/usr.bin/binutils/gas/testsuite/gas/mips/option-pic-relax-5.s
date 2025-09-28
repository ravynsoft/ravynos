	.text
	.ent	foo
	.option	pic0
foo:
	move	$2, $4
	move	$3, $5
	b	bar
	nop
	.space	0x7ffe << 1
bar:
	jr	$31
	.option	pic2
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
