	.text
foo:
	li	$5, %hi(.L0)
	sll	$5, 16
	addiu	$5, %lo(.L0)
.L0:
	b	.L0
	li	$5, %hi(.L0)
	sll	$5, 16
