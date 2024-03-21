	.syntax unified
	.text
	.arch armv7-a
	.arch_extension idiv

foo:
	udiv r0, r1, r2
	sdiv r0, r1, r2

	.thumb
	.thumb_func
bar:
	udiv r0, r1, r2
	sdiv r0, r1, r2
