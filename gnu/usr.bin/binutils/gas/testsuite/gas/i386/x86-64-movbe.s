# Check 64bit movbe
	.text
foo:
	movbe	(%r9),%r13w
	movbe	(%r9),%r13d
	movbe	(%r9),%r13
	movbe	%r13w,(%r9)
	movbe	%r13d,(%r9)
	movbe	%r13,(%r9)
	movbew	(%r9),%r13w
	movbel	(%r9),%r13d
	movbeq	(%r9),%r13
	movbew	%r13w,(%r9)
	movbel	%r13d,(%r9)
	movbeq	%r13,(%r9)

	.intel_syntax noprefix
	movbe bx, word ptr [rcx]
	movbe ebx, dword ptr [rcx]
	movbe rbx, qword ptr [rcx]
	movbe word ptr [rcx], bx
	movbe dword ptr [rcx], ebx
	movbe qword ptr [rcx], rbx
