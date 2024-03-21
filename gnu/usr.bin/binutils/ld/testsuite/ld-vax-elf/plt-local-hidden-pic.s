	.text

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
