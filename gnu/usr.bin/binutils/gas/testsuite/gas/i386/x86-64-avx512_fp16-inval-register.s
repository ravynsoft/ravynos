# Check error for destination and source operands have the same register .

	.allow_index_reg
	.text
_start:
	vfcmaddcph 8128(%rcx), %zmm29, %zmm29
	vfcmaddcph {rn-sae}, %zmm3, %zmm1, %zmm3
	vfcmaddcph %xmm0, %xmm1, %xmm0
	vfcmaddcsh %xmm0, %xmm0, %xmm0
	vfmaddcph %xmm1, %xmm0, %xmm0
	vfmaddcsh %xmm0, %xmm1, %xmm0
	vfcmulcph %xmm0, %xmm1, %xmm0
	vfcmulcsh %xmm0, %xmm1, %xmm0
	vfmulcph %xmm0, %xmm1, %xmm0
	vfmulcsh %xmm0, %xmm1, %xmm0
