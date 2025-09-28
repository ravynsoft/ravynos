# Check illegal movbe in 64bit mode.
	.text
foo:
	movbe	(%rcx),%bl
	movbe	%ecx,%ebx
	movbe	%bx,%rcx
	movbe	%rbx,%rcx
	movbe	%bl,(%rcx)

	.intel_syntax noprefix
	movbe bl, byte ptr [rcx]
	movbe ebx, ecx
	movbe rcx, bx
	movbe rcx, rbx
	movbe byte ptr [rcx], bl
