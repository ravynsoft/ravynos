	.data
start:
	.word	1
a:
	.tprelword t1
	.word	2
	.word	a-start
b:
	.dtprelword t2
	.word	3
	.word	b-start
	.word	0
