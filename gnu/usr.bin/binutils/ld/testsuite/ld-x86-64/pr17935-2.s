	.text
foo:
	.byte 0
	.globl	bar
bar:
	.dc.a	foo
