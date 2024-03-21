	.text
	.globl	_start
	.type	_start, @function
_start:
        lw	$2, %gp_rel(i)($28)

	.section .sdata
	.space 0x10000

	.section .sbss
i:
	.space	4
