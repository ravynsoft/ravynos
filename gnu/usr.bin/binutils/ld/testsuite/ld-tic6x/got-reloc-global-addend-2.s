.text
.nocmp
	ldw .d2t2 *+B14($DSBT_index(__c6xabi_DSBT_BASE)), B14
	mvkl .s1 $DPR_GOT(a)+4, A1

	.global a
	.section	.neardata,"aw",@progbits
	.align	2
	.type	a, @object
	.size	a, 4
a:
