# Test .arch .nommx
	.text
	.arch generic32
	emms
	.arch .mmx
	movd	%mm0, %eax
	femms
	.arch .3dnow
	femms
	pswapd	%mm1,%mm0
	.arch .3dnowa
	pswapd	%mm1,%mm0
	.arch .nommx
	emms
	.p2align 4
