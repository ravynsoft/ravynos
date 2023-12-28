	.weak	g
	.weak	g1
.text
	b .s2		g
[a2]	b .s2		g
	b .s2		g
||	mvk .s1		0, a1
	ldw .d2t2	*+B14(g1), B1
	mvkl	.s1	g, a1
	mvkh	.s1	g, a1

	.section	.data,"aw",@progbits
	.align	2
	.type	a, @object
	.size	a, 4
a:
	.long	g1
	.short	g1
	.byte	g1
