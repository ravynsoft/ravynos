	.text

	.globl	foo_global
	.type	foo_global, @function
foo_global:
	.word	0
	calls	$0, foo_extern
	calls	$0, foo_global
	calls	$0, foo_local
	calls	$0, foo_hidden
	calls	$0, foo_rehidden
	ret
	.size	foo_global, . - foo_global

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
