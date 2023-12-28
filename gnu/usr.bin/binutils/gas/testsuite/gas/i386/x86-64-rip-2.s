	.text
_start:
	movq test1(%rip), %rax
	.set test1, . + 0x7fffffff

	movq test2(%rip), %rax
	.set test2, . - 0x80000000

	.set test3, . + 0xf0000000
	.set test4, . - 0xf0000000
