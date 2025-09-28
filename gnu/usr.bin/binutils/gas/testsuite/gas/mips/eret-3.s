	.globl	foo
	.ent	foo
foo:
	eret
	.end	foo

	.globl	bar
	.ent	bar
bar:
	beq	$4,$0,1f
	sw	$4,0($5)
1:
	jr	$31
	.end	bar

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
        .space	8
