# Check warnings for invalid usage of register group

.text
	v4fmaddps (%rax), %zmm0, %zmm10
	v4fmaddps (%rax), %zmm1, %zmm10
	v4fmaddps (%rax), %zmm2, %zmm10
	v4fmaddps (%rax), %zmm3, %zmm10
	v4fmaddps (%rax), %zmm4, %zmm10
	v4fnmaddps (%rax), %zmm0, %zmm10
	v4fnmaddps (%rax), %zmm1, %zmm10
	v4fnmaddps (%rax), %zmm2, %zmm10
	v4fnmaddps (%rax), %zmm3, %zmm10
	v4fnmaddps (%rax), %zmm4, %zmm10
