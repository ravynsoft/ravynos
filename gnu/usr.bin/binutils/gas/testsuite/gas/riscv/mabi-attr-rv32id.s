	.attribute arch,"rv32ifd"
	.option pic
	.extern foo
	.text
foo:
	la	a0, foo
