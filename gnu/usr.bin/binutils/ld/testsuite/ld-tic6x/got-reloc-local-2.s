.text
.nocmp
	ldw .d2t2 *+B14($DSBT_index(__c6xabi_DSBT_BASE)), B14
	ldw .d2t1 *+B14($GOT(b)), A1

.data
b:
	.word 0
