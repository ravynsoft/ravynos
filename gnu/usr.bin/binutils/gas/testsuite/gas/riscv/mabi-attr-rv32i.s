	.attribute arch,"rv32i"
	.option pic
	.extern foo
	.text
foo:
	la	a0, foo
