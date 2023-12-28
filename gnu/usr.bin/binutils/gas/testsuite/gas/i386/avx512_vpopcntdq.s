# Check 32bit AVX512_VPOPCNTDQ instructions

	.allow_index_reg
	.text
_start:
	vpopcntd	%zmm5, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	%zmm5, %zmm6{%k7}	 # AVX512_VPOPCNTDQ
	vpopcntd	%zmm5, %zmm6{%k7}{z}	 # AVX512_VPOPCNTDQ
	vpopcntd	(%ecx), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	-123456(%esp,%esi,8), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	(%eax){1to16}, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	8128(%edx), %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	8192(%edx), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	-8192(%edx), %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	-8256(%edx), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	508(%edx){1to16}, %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	512(%edx){1to16}, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntd	-512(%edx){1to16}, %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	-516(%edx){1to16}, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	%zmm5, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	%zmm5, %zmm6{%k7}	 # AVX512_VPOPCNTDQ
	vpopcntq	%zmm5, %zmm6{%k7}{z}	 # AVX512_VPOPCNTDQ
	vpopcntq	(%ecx), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	-123456(%esp,%esi,8), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	(%eax){1to8}, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	8128(%edx), %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	8192(%edx), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	-8192(%edx), %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	-8256(%edx), %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	1016(%edx){1to8}, %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	1024(%edx){1to8}, %zmm6	 # AVX512_VPOPCNTDQ
	vpopcntq	-1024(%edx){1to8}, %zmm6	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	-1032(%edx){1to8}, %zmm6	 # AVX512_VPOPCNTDQ

	.intel_syntax noprefix
	vpopcntd	zmm6, zmm5	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6{k7}, zmm5	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6{k7}{z}, zmm5	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, ZMMWORD PTR [ecx]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, [eax]{1to16}	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, DWORD BCST [eax]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, [edx+508]{1to16}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm6, [edx+512]{1to16}	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm6, [edx-512]{1to16}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm6, [edx-516]{1to16}	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, zmm5	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6{k7}, zmm5	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6{k7}{z}, zmm5	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, ZMMWORD PTR [ecx]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, [eax]{1to8}	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, QWORD BCST [eax]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, [edx+1016]{1to8}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm6, [edx+1024]{1to8}	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm6, [edx-1024]{1to8}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm6, [edx-1032]{1to8}	 # AVX512_VPOPCNTDQ
