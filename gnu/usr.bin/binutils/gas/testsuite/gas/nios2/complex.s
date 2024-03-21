foo:
	ldw r2, (2-3)(r3)
	ldw r2, 2 + (2-3)(r3)
	ldw r2, 2 + (stack_top-3)(r3)

