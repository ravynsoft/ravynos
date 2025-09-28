# Check 64bit instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	vmovdqa32	%ymm1, %ymm2
