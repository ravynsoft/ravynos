	.section .text.foo,"axG",%progbits,foo_group
	.word 0

	.section .rodata.str.1,"aMSG",%progbits,1,foo_group
	.asciz "abc"

	.section .data.foo,"waG",%progbits,foo_group
	.word 1

	.section .dropme,"G",%progbits,foo_group
	.word 2

	.section .keepme,"G",%progbits,foo_group
	.word 3
