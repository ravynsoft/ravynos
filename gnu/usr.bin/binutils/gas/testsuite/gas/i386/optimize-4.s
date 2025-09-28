# Check instructions with optimized encoding

.include "optimize-1.s"

	{evex} vandnpd %zmm1, %zmm1, %zmm5
	{evex} vandnpd %ymm1, %ymm1, %ymm5
