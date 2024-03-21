	.syntax unified
	.text
	.thumb

	.globl	Strong1
	.thumb_func
	.type	Strong1, %function
Strong1:
	b	Strong2(PLT)
	b.w	Strong2(PLT)
	b.n	Strong2(PLT)
	b	Strong2
	b.w	Strong2
	b.n	Strong2
	.size	Strong1,.-Strong1

	.globl	Strong2
	.thumb_func
	.type	Strong2, %function
Strong2:
	b	Strong1(PLT)
	b.w	Strong1(PLT)
	b.n	Strong1(PLT)
	b	Strong1
	b.w	Strong1
	b.n	Strong1
	.size	Strong2, .-Strong2
