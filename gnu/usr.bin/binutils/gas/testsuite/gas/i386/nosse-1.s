# Test .arch .nosse
	.text
	.arch generic32
	lfence
	.arch .sse2
	lfence
	.arch .nosse
	lfence
	.p2align 4
