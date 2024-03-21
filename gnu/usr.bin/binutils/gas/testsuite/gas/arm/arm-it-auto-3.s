	.syntax unified
	.thumb
	.section .text.one
	cmp	r0, #0
	addeq	r1, #2
	.data
	.word	33
	.section .text.two
	addeq	r1, #3

