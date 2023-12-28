# Check RdRnd new instructions.

	.text
foo:
	rdrand %bx
	rdrand %ebx

	.intel_syntax noprefix
	rdrand bx
	rdrand ebx
