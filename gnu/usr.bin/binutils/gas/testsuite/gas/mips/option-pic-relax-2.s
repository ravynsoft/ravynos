	.text
	.ent	foo
	.option	pic2
foo:
	.set	noreorder
	.cpload	$25
	.set	reorder
	move	$2, $4
	move	$3, $5
	b	bar
	.space	0x7fff << 2
bar:
	jr	$31
	.option	pic0
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
