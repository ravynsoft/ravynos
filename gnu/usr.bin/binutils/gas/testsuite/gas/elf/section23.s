  .section	.data.retain_var,"0x200003"
	.global	retain_var
	.type	retain_var, %object
retain_var:
	.long	2

	.section	.text._start,"ax"
	.global	_start
	.type	_start, %function
_start:
	.word 0
