	.text
	.type	resolve_do_it, %function
resolve_do_it:
	.byte 0
	.size	resolve_do_it, .-resolve_do_it
	.globl	do_it
	.type	do_it, %gnu_indirect_function
	.set	do_it,resolve_do_it
