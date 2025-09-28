	.text
	.space 13
	.globl  foo
	.type   foo, %function
foo:
	.byte 0
	.symver foo, foo@VERS_2
	.symver foo, foo@@VERS_2
	.symver foo, foo@VERS_1
	.hidden foo
	.size foo,.-foo
