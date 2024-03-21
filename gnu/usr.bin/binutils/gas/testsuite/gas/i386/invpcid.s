# Check INVPCID instruction

	.text
foo:
	.rept 2

	invpcid	(%eax), %edx

	.intel_syntax noprefix
	invpcid	edx,[eax]
	invpcid	edx,oword ptr [eax]

	.att_syntax prefix
	.code16
	.endr
