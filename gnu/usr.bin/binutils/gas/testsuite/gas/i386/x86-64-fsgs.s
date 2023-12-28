# Check 64bit FSGS new instructions.

	.text
foo:
	rdfsbase %ebx
	rdfsbase %rbx
	rdfsbase %r8d
	rdfsbase %r8
	rdgsbase %ebx
	rdgsbase %rbx
	rdgsbase %r8d
	rdgsbase %r8
	wrfsbase %ebx
	wrfsbase %rbx
	wrfsbase %r8d
	wrfsbase %r8
	wrgsbase %ebx
	wrgsbase %rbx
	wrgsbase %r8d
	wrgsbase %r8

	.intel_syntax noprefix
	rdfsbase ebx
	rdfsbase rbx
	rdfsbase r8d
	rdfsbase r8
	rdgsbase ebx
	rdgsbase rbx
	rdgsbase r8d
	rdgsbase r8
	wrfsbase ebx
	wrfsbase rbx
	wrfsbase r8d
	wrfsbase r8
	wrgsbase ebx
	wrgsbase rbx
	wrgsbase r8d
	wrgsbase r8
