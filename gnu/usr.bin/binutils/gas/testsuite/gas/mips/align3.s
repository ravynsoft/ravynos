	.data
start:
	.byte	0
foo:
bar:
	.align	2
	.word	foo - start
	.word	bar - start
	.word	0
