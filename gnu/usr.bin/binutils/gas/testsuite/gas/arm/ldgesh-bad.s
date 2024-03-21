.syntax unified
.arch armv7-a
.thumb
	.global foo
foo:
	it ge
	ldgesh r1, [r11, #4]
