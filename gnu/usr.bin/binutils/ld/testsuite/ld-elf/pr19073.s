	.text
	.globl	__foo
	.type	__foo, %function
__foo:
	.byte 0
	.globl	foo
	.weak foo
	.set foo, __foo
	.symver __foo,foo@@VERS.1
