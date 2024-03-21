	.text
test:
	move	$16, $2
	jal	func
	addiu	$16, $17, 1
	jal	func
	addiu	$16, $17, foo
	jal	func
	addiu	$16, $17, 0x3fff
	jal	func
	addiu	$16, $17, bar
	jal	func

	move	$16, $2
	jr	$31
	addiu	$16, $17, 1
	jr	$31
	addiu	$16, $17, foo
	jr	$31
	addiu	$16, $17, 0x3fff
	jr	$31
	addiu	$16, $17, bar
	jr	$31

	.set	foo, 1
	.set	bar, 0x3fff

	# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
