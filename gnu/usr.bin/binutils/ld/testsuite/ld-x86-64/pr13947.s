	.text
	.globl foo
	.type foo, @function
foo:
	call bar@PLT
