	.global a
	.text
	.nocmp
fish:
	ldw .d2t2 *+B14($DSBT_index(__c6xabi_DSBT_BASE)), B14
	callp .s2 sub0, B3
	b .s2 sub0
	callp .s2 fish, B3
	b .s2 fish
	ldw .d2t2 *+B14(a), B4
	ldw .d2t2 *+B14(b), B5
	ldw .d2t2 *+B14($GOT(a)), B6
	ldw .d2t2 *+B14($GOT(b)), B6
	mvkl .s1 $DPR_GOT(a), A4
	mvkh .s1 $DPR_GOT(a), A4
	mvkl .s1 $DPR_GOT(b), A5
	mvkh .s1 $DPR_GOT(b), A5
	.global	b
	.section	.neardata,"aw",@progbits
	.align	2
	.type	b, @object
	.size	b, 4
b:
	.long	0x12345678
	.weak	g1
	.weak	g2
	.type	w, @object
	.size	w, 8
w:
	.long	g1
	.long	g2
	.section	.note.GNU-stack
