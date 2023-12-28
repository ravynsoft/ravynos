# Check 64bit AVX512_VPOPCNTDQ instructions

	.allow_index_reg
	.text
_start:
	vpopcntd	%zmm29, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	%zmm29, %zmm30{%k7}	 # AVX512_VPOPCNTDQ
	vpopcntd	%zmm29, %zmm30{%k7}{z}	 # AVX512_VPOPCNTDQ
	vpopcntd	(%rcx), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	0x123(%rax,%r14,8), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	(%rcx){1to16}, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	8128(%rdx), %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	8192(%rdx), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	-8192(%rdx), %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	-8256(%rdx), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	508(%rdx){1to16}, %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	512(%rdx){1to16}, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntd	-512(%rdx){1to16}, %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	-516(%rdx){1to16}, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	%zmm29, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	%zmm29, %zmm30{%k7}	 # AVX512_VPOPCNTDQ
	vpopcntq	%zmm29, %zmm30{%k7}{z}	 # AVX512_VPOPCNTDQ
	vpopcntq	(%rcx), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	0x123(%rax,%r14,8), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	(%rcx){1to8}, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	8128(%rdx), %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	8192(%rdx), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	-8192(%rdx), %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	-8256(%rdx), %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	1016(%rdx){1to8}, %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	1024(%rdx){1to8}, %zmm30	 # AVX512_VPOPCNTDQ
	vpopcntq	-1024(%rdx){1to8}, %zmm30	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	-1032(%rdx){1to8}, %zmm30	 # AVX512_VPOPCNTDQ

	.intel_syntax noprefix
	vpopcntd	zmm30, zmm29	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30{k7}, zmm29	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30{k7}{z}, zmm29	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, ZMMWORD PTR [rcx]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, [rcx]{1to16}	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, [rdx+508]{1to16}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm30, [rdx+512]{1to16}	 # AVX512_VPOPCNTDQ
	vpopcntd	zmm30, [rdx-512]{1to16}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntd	zmm30, [rdx-516]{1to16}	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, zmm29	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30{k7}, zmm29	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30{k7}{z}, zmm29	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, ZMMWORD PTR [rcx]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, [rcx]{1to8}	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, [rdx+1016]{1to8}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm30, [rdx+1024]{1to8}	 # AVX512_VPOPCNTDQ
	vpopcntq	zmm30, [rdx-1024]{1to8}	 # AVX512_VPOPCNTDQ Disp8
	vpopcntq	zmm30, [rdx-1032]{1to8}	 # AVX512_VPOPCNTDQ
