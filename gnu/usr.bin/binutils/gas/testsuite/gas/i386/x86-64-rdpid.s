# Check 64bit RDPID instructions.

	.text
_start:
	rdpid %rax
	rdpid %r10
