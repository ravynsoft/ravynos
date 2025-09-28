# Check 64bit RdRnd new instructions.

	.text
foo:
	rdrand %bx
	rdrand %ebx
	rdrand %rbx
	rdrand %r8w
	rdrand %r8d
	rdrand %r8

	.intel_syntax noprefix
	rdrand bx
	rdrand ebx
	rdrand rbx
	rdrand r8w
	rdrand r8d
	rdrand r8
