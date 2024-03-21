	.text
	.align 4
	.global foo
foo:	entry	sp, 16
	.literal .Lit0, foo@pcrel
1:	l32r	a2, .Lit0
.L2:	.word	foo@pcrel
	.long	1b@pcrel
	.4byte	.L2@pcrel
