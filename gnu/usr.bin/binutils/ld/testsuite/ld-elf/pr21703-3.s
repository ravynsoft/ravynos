	.text
	.global foo
	.type	foo, %function
foo:
	.space	4
	.size	foo, 4

	.global foo1
	.type	foo1, %function
foo1:
	.space	32
	.size	foo1, 32

	.symver	foo, foo@FOO
	.symver	foo1, foo@@FOO1
