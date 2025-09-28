	.include "tocopt6-inc.s"
	in123
	in23
	gobblegot f3

	.section .text
	.globl main
	start main
	bl f1
	nop
	bl f2
	nop
	bl f3
	nop
	bl g2
	nop
	bl in23
	nop
	li 3, 0
	end main
