	.text
	.globl	foo
	.ent	foo
foo:
	lui	$2, %hi(bar)
	addiu	$2, %lo(bar)
	lui	$2, %hi(bar)
	addiu	$2, %lo(bar)
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
