# Check illegal movbe
	.text
foo:
	movbe	(%ecx),%bl
	movbe	%ecx,%ebx
	movbe	%bx,%ecx
	movbe	%bl,(%ecx)

	.intel_syntax noprefix
	movbe bl, byte ptr [ecx]
	movbe ebx, ecx
	movbe ecx, bx
	movbe byte ptr [ecx], bl
