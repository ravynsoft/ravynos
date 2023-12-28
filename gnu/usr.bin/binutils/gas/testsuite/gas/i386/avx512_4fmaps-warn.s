# Check warnings for invalid usage of register group

.text
	v4fmaddps (%eax), %zmm0, %zmm6
	v4fmaddps (%eax), %zmm1, %zmm6
	v4fmaddps (%eax), %zmm2, %zmm6
	v4fmaddps (%eax), %zmm3, %zmm6
	v4fmaddps (%eax), %zmm4, %zmm6
	v4fnmaddps (%eax), %zmm0, %zmm6
	v4fnmaddps (%eax), %zmm1, %zmm6
	v4fnmaddps (%eax), %zmm2, %zmm6
	v4fnmaddps (%eax), %zmm3, %zmm6
	v4fnmaddps (%eax), %zmm4, %zmm6
	v4fmaddss (%eax), %xmm0, %xmm6
	v4fmaddss (%eax), %xmm1, %xmm6
	v4fmaddss (%eax), %xmm2, %xmm6
	v4fmaddss (%eax), %xmm3, %xmm6
	v4fmaddss (%eax), %xmm4, %xmm6
	v4fnmaddss (%eax), %xmm0, %xmm6
	v4fnmaddss (%eax), %xmm1, %xmm6
	v4fnmaddss (%eax), %xmm2, %xmm6
	v4fnmaddss (%eax), %xmm3, %xmm6
	v4fnmaddss (%eax), %xmm4, %xmm6
