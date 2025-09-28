	.arch armv5te

	.text
	.align 2
	.code 32
	.type myfunction, %function
myfunction:
	bx r14

	.text
	.align 2
	.code 16
	.thumb_func
	.global caller
	.type caller, %function
caller:
	nop
	bl myfunction
	nop
	bl myfunction
	nop
	bl myfunction
	nop
	bl myfunction

	.text
	.align 2
	.code 16
	.type mythumbfunction, %function
	.thumb_func
mythumbfunction:
	bx r14

	.text
	.align 2
	.code 32
	.global armcaller
	.type armcaller, %function
armcaller:
	bl mythumbfunction

