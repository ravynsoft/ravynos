# Check RTM new instructions.

	.text
foo:
	xabort $0x8
1:
	xbegin 1b
	xbegin 2f
2:
	xend

	.intel_syntax noprefix
	xabort 0x8
1:
	xbegin 1b
	xbegin 2f
2:
	xend
	xtest
