# Check 32bit RDPID instructions.

	.text
_start:
	rdpid %eax

	.code16
	rdpid %ecx
