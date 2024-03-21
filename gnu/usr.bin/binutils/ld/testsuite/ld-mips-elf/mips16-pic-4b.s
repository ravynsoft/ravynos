	.abicalls
	.set	noreorder

	# Define a MIPS16 function f1@@V1.

	.global	_f1
	.symver	_f1,f1@@V1

	.set	mips16
	.type	_f1,@function
	.ent	_f1
_f1:
	jr	$31
	nop
	.end	_f1
