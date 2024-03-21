  .section	.data.var,"0x800003"
	.global	var
	.type	var, %object
var:
	.long	2

	.section	.text._start,"ax"
	.global	_start
	.type	_start, %function
_start:
	.word 0
