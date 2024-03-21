# Test .arch [.sseX|.nosseX]
	.text
	.arch generic32
	.arch .mmx
	emms
	addps %xmm0, %xmm0
	.arch .sse
	addps %xmm0, %xmm0
	lfence
	.arch .sse2
	lfence
	mwait
	.arch .sse3
	mwait
	pabsd %xmm0, %xmm0
	.arch .ssse3
	pabsd %xmm0, %xmm0
	ptest %xmm0, %xmm0
	.arch .sse4.1
	ptest %xmm0, %xmm0
	crc32 %eax, %eax
	.arch .sse4.2
	crc32 %eax, %eax
	.arch .nosse
	.arch .sse4
	crc32 %eax, %eax
	.arch .nosse4
	ptest %xmm0, %xmm0
	pabsd %xmm0, %xmm0
	.arch .sse4
	.arch .nosse4.2
	crc32 %eax, %eax
	ptest %xmm0, %xmm0
	.arch .nosse4.1
	ptest %xmm0, %xmm0
	pabsd %xmm0, %xmm0
	.arch .nossse3
	pabsd %xmm0, %xmm0
	emms
	.arch .nommx
	.arch .nosse3
	lfence
	emms
	.arch .nosse2
	lfence
	addps %xmm0, %xmm0
	.arch .nosse
	addps %xmm0, %xmm0
	.p2align 4
