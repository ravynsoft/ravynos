	.data
	.globl foo
	.type foo,%object
foo:
	.byte 0
	.size foo,.-foo
	.symver foo,foo@@version2,remove
	.symver foo,foo@version1
