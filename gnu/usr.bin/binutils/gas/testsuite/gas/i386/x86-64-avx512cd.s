# Check 64bit AVX512CD instructions

	.allow_index_reg
	.text
_start:

	vpconflictd	%zmm29, %zmm30	 # AVX512CD
	vpconflictd	%zmm29, %zmm30{%k7}	 # AVX512CD
	vpconflictd	%zmm29, %zmm30{%k7}{z}	 # AVX512CD
	vpconflictd	(%rcx), %zmm30	 # AVX512CD
	vpconflictd	0x123(%rax,%r14,8), %zmm30	 # AVX512CD
	vpconflictd	(%rcx){1to16}, %zmm30	 # AVX512CD
	vpconflictd	8128(%rdx), %zmm30	 # AVX512CD Disp8
	vpconflictd	8192(%rdx), %zmm30	 # AVX512CD
	vpconflictd	-8192(%rdx), %zmm30	 # AVX512CD Disp8
	vpconflictd	-8256(%rdx), %zmm30	 # AVX512CD
	vpconflictd	508(%rdx){1to16}, %zmm30	 # AVX512CD Disp8
	vpconflictd	512(%rdx){1to16}, %zmm30	 # AVX512CD
	vpconflictd	-512(%rdx){1to16}, %zmm30	 # AVX512CD Disp8
	vpconflictd	-516(%rdx){1to16}, %zmm30	 # AVX512CD

	vpconflictq	%zmm29, %zmm30	 # AVX512CD
	vpconflictq	%zmm29, %zmm30{%k7}	 # AVX512CD
	vpconflictq	%zmm29, %zmm30{%k7}{z}	 # AVX512CD
	vpconflictq	(%rcx), %zmm30	 # AVX512CD
	vpconflictq	0x123(%rax,%r14,8), %zmm30	 # AVX512CD
	vpconflictq	(%rcx){1to8}, %zmm30	 # AVX512CD
	vpconflictq	8128(%rdx), %zmm30	 # AVX512CD Disp8
	vpconflictq	8192(%rdx), %zmm30	 # AVX512CD
	vpconflictq	-8192(%rdx), %zmm30	 # AVX512CD Disp8
	vpconflictq	-8256(%rdx), %zmm30	 # AVX512CD
	vpconflictq	1016(%rdx){1to8}, %zmm30	 # AVX512CD Disp8
	vpconflictq	1024(%rdx){1to8}, %zmm30	 # AVX512CD
	vpconflictq	-1024(%rdx){1to8}, %zmm30	 # AVX512CD Disp8
	vpconflictq	-1032(%rdx){1to8}, %zmm30	 # AVX512CD

	vplzcntd	%zmm29, %zmm30	 # AVX512CD
	vplzcntd	%zmm29, %zmm30{%k7}	 # AVX512CD
	vplzcntd	%zmm29, %zmm30{%k7}{z}	 # AVX512CD
	vplzcntd	(%rcx), %zmm30	 # AVX512CD
	vplzcntd	0x123(%rax,%r14,8), %zmm30	 # AVX512CD
	vplzcntd	(%rcx){1to16}, %zmm30	 # AVX512CD
	vplzcntd	8128(%rdx), %zmm30	 # AVX512CD Disp8
	vplzcntd	8192(%rdx), %zmm30	 # AVX512CD
	vplzcntd	-8192(%rdx), %zmm30	 # AVX512CD Disp8
	vplzcntd	-8256(%rdx), %zmm30	 # AVX512CD
	vplzcntd	508(%rdx){1to16}, %zmm30	 # AVX512CD Disp8
	vplzcntd	512(%rdx){1to16}, %zmm30	 # AVX512CD
	vplzcntd	-512(%rdx){1to16}, %zmm30	 # AVX512CD Disp8
	vplzcntd	-516(%rdx){1to16}, %zmm30	 # AVX512CD

	vplzcntq	%zmm29, %zmm30	 # AVX512CD
	vplzcntq	%zmm29, %zmm30{%k7}	 # AVX512CD
	vplzcntq	%zmm29, %zmm30{%k7}{z}	 # AVX512CD
	vplzcntq	(%rcx), %zmm30	 # AVX512CD
	vplzcntq	0x123(%rax,%r14,8), %zmm30	 # AVX512CD
	vplzcntq	(%rcx){1to8}, %zmm30	 # AVX512CD
	vplzcntq	8128(%rdx), %zmm30	 # AVX512CD Disp8
	vplzcntq	8192(%rdx), %zmm30	 # AVX512CD
	vplzcntq	-8192(%rdx), %zmm30	 # AVX512CD Disp8
	vplzcntq	-8256(%rdx), %zmm30	 # AVX512CD
	vplzcntq	1016(%rdx){1to8}, %zmm30	 # AVX512CD Disp8
	vplzcntq	1024(%rdx){1to8}, %zmm30	 # AVX512CD
	vplzcntq	-1024(%rdx){1to8}, %zmm30	 # AVX512CD Disp8
	vplzcntq	-1032(%rdx){1to8}, %zmm30	 # AVX512CD

	vpbroadcastmw2d	%k6, %zmm30	 # AVX512CD

	vpbroadcastmb2q	%k6, %zmm30	 # AVX512CD

	.intel_syntax noprefix
	vpconflictd	zmm30, zmm29	 # AVX512CD
	vpconflictd	zmm30{k7}, zmm29	 # AVX512CD
	vpconflictd	zmm30{k7}{z}, zmm29	 # AVX512CD
	vpconflictd	zmm30, ZMMWORD PTR [rcx]	 # AVX512CD
	vpconflictd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512CD
	vpconflictd	zmm30, [rcx]{1to16}	 # AVX512CD
	vpconflictd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512CD Disp8
	vpconflictd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512CD
	vpconflictd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512CD Disp8
	vpconflictd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512CD
	vpconflictd	zmm30, [rdx+508]{1to16}	 # AVX512CD Disp8
	vpconflictd	zmm30, [rdx+512]{1to16}	 # AVX512CD
	vpconflictd	zmm30, [rdx-512]{1to16}	 # AVX512CD Disp8
	vpconflictd	zmm30, [rdx-516]{1to16}	 # AVX512CD

	vpconflictq	zmm30, zmm29	 # AVX512CD
	vpconflictq	zmm30{k7}, zmm29	 # AVX512CD
	vpconflictq	zmm30{k7}{z}, zmm29	 # AVX512CD
	vpconflictq	zmm30, ZMMWORD PTR [rcx]	 # AVX512CD
	vpconflictq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512CD
	vpconflictq	zmm30, [rcx]{1to8}	 # AVX512CD
	vpconflictq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512CD Disp8
	vpconflictq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512CD
	vpconflictq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512CD Disp8
	vpconflictq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512CD
	vpconflictq	zmm30, [rdx+1016]{1to8}	 # AVX512CD Disp8
	vpconflictq	zmm30, [rdx+1024]{1to8}	 # AVX512CD
	vpconflictq	zmm30, [rdx-1024]{1to8}	 # AVX512CD Disp8
	vpconflictq	zmm30, [rdx-1032]{1to8}	 # AVX512CD

	vplzcntd	zmm30, zmm29	 # AVX512CD
	vplzcntd	zmm30{k7}, zmm29	 # AVX512CD
	vplzcntd	zmm30{k7}{z}, zmm29	 # AVX512CD
	vplzcntd	zmm30, ZMMWORD PTR [rcx]	 # AVX512CD
	vplzcntd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512CD
	vplzcntd	zmm30, [rcx]{1to16}	 # AVX512CD
	vplzcntd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512CD Disp8
	vplzcntd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512CD
	vplzcntd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512CD Disp8
	vplzcntd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512CD
	vplzcntd	zmm30, [rdx+508]{1to16}	 # AVX512CD Disp8
	vplzcntd	zmm30, [rdx+512]{1to16}	 # AVX512CD
	vplzcntd	zmm30, [rdx-512]{1to16}	 # AVX512CD Disp8
	vplzcntd	zmm30, [rdx-516]{1to16}	 # AVX512CD

	vplzcntq	zmm30, zmm29	 # AVX512CD
	vplzcntq	zmm30{k7}, zmm29	 # AVX512CD
	vplzcntq	zmm30{k7}{z}, zmm29	 # AVX512CD
	vplzcntq	zmm30, ZMMWORD PTR [rcx]	 # AVX512CD
	vplzcntq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512CD
	vplzcntq	zmm30, [rcx]{1to8}	 # AVX512CD
	vplzcntq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512CD Disp8
	vplzcntq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512CD
	vplzcntq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512CD Disp8
	vplzcntq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512CD
	vplzcntq	zmm30, [rdx+1016]{1to8}	 # AVX512CD Disp8
	vplzcntq	zmm30, [rdx+1024]{1to8}	 # AVX512CD
	vplzcntq	zmm30, [rdx-1024]{1to8}	 # AVX512CD Disp8
	vplzcntq	zmm30, [rdx-1032]{1to8}	 # AVX512CD

	vpbroadcastmw2d	zmm30, k6	 # AVX512CD

	vpbroadcastmb2q	zmm30, k6	 # AVX512CD

