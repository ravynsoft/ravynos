	.comm	DEBUGLEVEL,4,4
	.text
	.globl	_start
	.type	_start, @function
_start:
	.byte 0xeb, 0x8b
	movl      $DEBUGLEVEL@GOT, %ebp
