# Check movbe
	.text
foo:
	movbe	(%ecx),%bx
	movbe	(%ecx),%ebx
	movbe	%bx,(%ecx)
	movbe	%ebx,(%ecx)
	movbew	(%ecx),%bx
	movbel	(%ecx),%ebx
	movbew	%bx,(%ecx)
	movbel	%ebx,(%ecx)

	.intel_syntax noprefix
	movbe bx, word ptr [ecx]
	movbe ebx, dword ptr [ecx]
	movbe word ptr [ecx], bx
	movbe dword ptr [ecx], ebx
