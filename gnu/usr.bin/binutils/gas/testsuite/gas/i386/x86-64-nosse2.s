# Test .arch .nosse2
	.text
	.arch generic64
	.arch .nosse2
	addps %xmm0, %xmm0
	paddb %xmm0, %xmm0
	movq %xmm0, %rax
	movq %rax, %xmm0
	.p2align 4
