	.text
_start:
	movq test1(%rip), %rax
	.set test1, . + 0x80000000
	.end
