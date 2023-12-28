	.text
	.globl	_start
_start:
	lda	$0, foo($29)	!gprel

	.section	.sbss
foo:	.quad	0
