	.text
	.arch corei7
_start:
	.arch .avx
	vpconflictd	%xmm0, %xmm25 
	vpconflictd	%ymm0, %ymm25 
	vpconflictd	%ymm0, %zmm25 
	.arch .avx512vl
	vpconflictd	%xmm0, %xmm25 
	vpconflictd	%ymm0, %ymm25 
	vpconflictd	%zmm0, %zmm25 
	.arch .avx512cd
	vpconflictd	%xmm0, %xmm25 
	vpconflictd	%ymm0, %ymm25 
	vpconflictd	%zmm0, %zmm25 
	.p2align 4
