	.section .text.foo,"axG",%progbits,foo_group
	.word 0

	.section .data.foo,"waG",%progbits,foo_group
	.word 1

	.section .text, "ax"
	.word 0

	.section .data, "wa"
	.word 1
