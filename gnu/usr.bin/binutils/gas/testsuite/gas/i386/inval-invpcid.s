# Check illegal INVPCID instructions
	.text
foo:
	invpcid	(%ecx), %bx
	invpcid %ebx, (%ecx)
	invpcid	%ebx, %ecx

	.intel_syntax noprefix
	invpcid bx, [ecx]
	invpcid [ecx], ebx
	invpcid ecx, ebx
