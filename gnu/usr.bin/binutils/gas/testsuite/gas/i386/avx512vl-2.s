	.text
	.arch corei7
_start:
	.arch .avx
	vpconflictd	%xmm0, %xmm5 
	vpconflictd	%ymm0, %ymm5 
	vpconflictd	%ymm0, %zmm5 
	.arch .avx512vl
	vpconflictd	%xmm0, %xmm5 
	vpconflictd	%ymm0, %ymm5 
	vpconflictd	%zmm0, %zmm5 
	.arch .avx512cd
	vpconflictd	%xmm0, %xmm5 
	vpconflictd	%ymm0, %ymm5 
	vpconflictd	%zmm0, %zmm5 
	.p2align 4
