#AMDFAM10 New Instructions

	.text
foo:
	lzcnt	(%ecx),%ebx
	lzcnt	(%ecx),%bx
	lzcnt	%ecx,%ebx
	lzcnt	%cx,%bx
	popcnt	(%ecx),%ebx
	popcnt	(%ecx),%bx
	popcnt	%ecx,%ebx
	popcnt	%cx,%bx
	extrq	%xmm2,%xmm1
	extrq	$4,$2,%xmm1
	insertq	%xmm2,%xmm1
	insertq	$4,$2,%xmm2,%xmm1
	movntsd	%xmm1,(%ecx)
	movntss %xmm1,(%ecx)

	.intel_syntax noprefix
	lzcnt	ebx,[ecx]
	lzcnt	bx,[ecx]
	lzcnt	ebx,ecx
	lzcnt	bx,cx
	popcnt	ebx,[ecx]
	popcnt	bx,[ecx]
	popcnt	ebx,ecx
	popcnt	bx,cx
	extrq	xmm1,xmm2
	extrq	xmm1,2,4
	insertq	xmm1,xmm2
	insertq	xmm1,xmm2,2,4
	movntsd	[ecx],xmm1
	movntss [ecx],xmm1

	# Force a good alignment.
	.p2align	4,0
