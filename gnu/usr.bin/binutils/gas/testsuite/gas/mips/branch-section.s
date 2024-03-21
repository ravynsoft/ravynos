	.text
	.globl	foo
	.ent	foo
foo:
	b	bar
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16

	.section .init, "ax", @progbits
	.ent	bar
bar:
	jr	$ra
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16
