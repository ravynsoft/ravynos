	.text

	.globl	foo_extern
	.type	foo_extern, @function
foo_extern:
	.word	0
	calls	$0, foo_extern
	calls	$0, foo_global
	calls	$0, foo_local
	calls	$0, foo_hidden
	calls	$0, foo_rehidden
	ret
	.size	foo_extern, . - foo_extern

	.globl	foo_local
	.type	foo_local, @function
foo_local:
	.word	0
	calls	$0, foo_extern
	calls	$0, foo_global
	calls	$0, foo_local
	calls	$0, foo_hidden
	calls	$0, foo_rehidden
	ret
	.size	foo_local, . - foo_local

	.hidden	foo_hidden
	.globl	foo_hidden
	.type	foo_hidden, @function
foo_hidden:
	.word	0
	calls	$0, foo_extern
	calls	$0, foo_global
	calls	$0, foo_local
	calls	$0, foo_hidden
	calls	$0, foo_rehidden
	ret
	.size	foo_hidden, . - foo_hidden

	.globl	foo_rehidden
	.type	foo_rehidden, @function
foo_rehidden:
	.word	0
	calls	$0, foo_extern
	calls	$0, foo_global
	calls	$0, foo_local
	calls	$0, foo_hidden
	calls	$0, foo_rehidden
	ret
	.size	foo_rehidden, . - foo_rehidden
