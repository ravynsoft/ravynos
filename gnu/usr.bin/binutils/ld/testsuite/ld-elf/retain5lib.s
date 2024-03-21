/* The link will fail if foo is included because undefined_sym is not defined.  */
	.section	.text.foo,"axR"
	.global	foo
	.type	foo, %function
foo:
	.long undefined_sym
