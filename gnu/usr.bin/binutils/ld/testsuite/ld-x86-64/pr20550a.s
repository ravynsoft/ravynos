	.text
	.global _start
_start:
	movabsq	$strings@SIZE, %rdx
	.section	.data.rel,"aw",@progbits
	.quad	strings
	.section	.note.GNU-stack
