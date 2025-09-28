	.text
	.globl	foo
	.ent	foo
foo:
	b	bar
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16
