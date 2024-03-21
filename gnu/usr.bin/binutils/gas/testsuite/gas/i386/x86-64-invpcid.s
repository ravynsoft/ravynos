# Check 64bit INVPCID instruction

	.text
foo:
	invpcid	(%rax), %rdx

	.intel_syntax noprefix
	invpcid	rdx,[rax]
	invpcid	rdx,oword ptr [rax]
