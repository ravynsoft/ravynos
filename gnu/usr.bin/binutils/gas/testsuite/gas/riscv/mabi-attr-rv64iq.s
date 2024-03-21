	.attribute arch,"rv64ifdq"
	.option pic
	.extern foo
	.text
foo:
	la	a0, foo
