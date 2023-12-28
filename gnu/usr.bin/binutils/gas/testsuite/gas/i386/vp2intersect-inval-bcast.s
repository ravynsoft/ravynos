# Check error for invalid {1toXX} and {2toXX} broadcasts.

	.allow_index_reg
	.text
_start:
	vp2intersectd 8(%eax){1to8}, %xmm2, %k3
	vp2intersectd 8(%eax){1to16}, %ymm2, %k3
	vp2intersectd 8(%eax){1to8}, %zmm2, %k3
	vp2intersectq 8(%eax){1to4}, %xmm2, %k3
	vp2intersectq 8(%eax){1to8}, %ymm2, %k3
	vp2intersectq 8(%eax){1to16}, %zmm2, %k3

	.intel_syntax noprefix
	vp2intersectd k3, zmm2, 8[eax]{1to8}
	vp2intersectd k3, ymm2, 8[eax]{1to16}
	vp2intersectd k3, xmm2, 8[eax]{1to8}
	vp2intersectq k3, zmm2, 8[eax]{1to16}
	vp2intersectq k3, ymm2, 8[eax]{1to8}
	vp2intersectq k3, xmm2, 8[eax]{1to4}
