	.text
	.globl	foo
	.type	foo, @function
foo:
	.word	0
	calls	$0, protected_foo
	calls	$0, hidden_foo
	calls	$0, internal_foo
	ret
	.size	foo, . - foo
