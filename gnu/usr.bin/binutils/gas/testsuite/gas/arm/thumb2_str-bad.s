	.syntax unified
	.text
	.align	2
	.global	thumb2_str
	.thumb
	.thumb_func
thumb2_str:
	str r0, [pc, 4]
	str r0, d
	str pc, [r0]

	.space 4
	.align
d:
	.long 0
