#AMDFAM10 New Instructions

	.text
foo:
	lzcnt	(%rcx),%rbx
	lzcnt	(%rcx),%ebx
	lzcnt	(%rcx),%bx
	lzcnt	%rcx,%rbx
	lzcnt	%ecx,%ebx
	lzcnt	%cx,%bx
	popcnt	(%rcx),%rbx
	popcnt	(%rcx),%ebx
	popcnt	(%rcx),%bx
	popcnt	%rcx,%rbx
	popcnt	%ecx,%ebx
	popcnt	%cx,%bx
	extrq	%xmm2,%xmm1
	extrq	$4,$2,%xmm1
	insertq	%xmm2,%xmm1
	insertq	$4,$2,%xmm2,%xmm1
	movntsd	%xmm1,(%rcx)
	movntss %xmm1,(%rcx)

	.intel_syntax noprefix
	lzcnt	rbx,[rcx]
	lzcnt	ebx,[rcx]
	lzcnt	bx,[rcx]
	lzcnt	rbx,rcx
	lzcnt	ebx,ecx
	lzcnt	bx,cx
	popcnt	rbx,[rcx]
	popcnt	ebx,[rcx]
	popcnt	bx,[rcx]
	popcnt	rbx,rcx
	popcnt	ebx,ecx
	popcnt	bx,cx
	extrq	xmm1,xmm2
	extrq	xmm1,2,4
	insertq	xmm1,xmm2
	insertq	xmm1,xmm2,2,4
	movntsd	[rcx],xmm1
	movntss [rcx],xmm1

	# Force a good alignment.
	.p2align	4,0
