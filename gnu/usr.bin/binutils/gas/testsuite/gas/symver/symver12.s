	.text
	.space 13
	.symver foo, foo@@VERS_2
	.symver foo, foo@VERS_1
	.symver foo, foo@VERS_2
	.globl  foo
	.type   foo, %function
foo:
	.byte 0
	.size foo,.-foo
