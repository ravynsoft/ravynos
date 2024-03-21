	.cpu cortex-m3
	.text
	.align	1
	.global	bar
	.arch armv7-m
	.syntax unified
	.thumb
	.thumb_func
	.fpu softvfp
	.type	bar, %function
bar:
	push	{r7, lr}
	add	r7, sp, #0
	bl	foo(PLT)
	mov	r3, r0
	mov	r0, r3
	pop	{r7, pc}
	.size	bar, .-bar
