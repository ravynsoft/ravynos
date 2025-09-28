/* A section that doesn't match any linker script input section rules but
   has SHF_GNU_RETAIN applied should not be garbage collected.  */
	.section	.orphaned_section,"axR"
	.global	orphaned_fn
	.type	orphaned_fn, %function
orphaned_fn:
	.word 0

	.section	.text._start,"ax"
	.global	_start
	.type	_start, %function
_start:
	.word 0
