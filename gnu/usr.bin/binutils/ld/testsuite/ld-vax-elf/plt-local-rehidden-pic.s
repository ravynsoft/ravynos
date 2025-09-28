	.text

	.hidden	foo_rehidden
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
