	.data
	.global foo
foo:
	.byte 1
	.symver foo,foo@@@version1
	.symver foo,foo@@@version2
