# Check 32bit AVX512CD instructions

	.allow_index_reg
	.text
_start:

	vpconflictd	%zmm5, %zmm6	 # AVX512CD
	vpconflictd	%zmm5, %zmm6{%k7}	 # AVX512CD
	vpconflictd	%zmm5, %zmm6{%k7}{z}	 # AVX512CD
	vpconflictd	(%ecx), %zmm6	 # AVX512CD
	vpconflictd	-123456(%esp,%esi,8), %zmm6	 # AVX512CD
	vpconflictd	(%eax){1to16}, %zmm6	 # AVX512CD
	vpconflictd	8128(%edx), %zmm6	 # AVX512CD Disp8
	vpconflictd	8192(%edx), %zmm6	 # AVX512CD
	vpconflictd	-8192(%edx), %zmm6	 # AVX512CD Disp8
	vpconflictd	-8256(%edx), %zmm6	 # AVX512CD
	vpconflictd	508(%edx){1to16}, %zmm6	 # AVX512CD Disp8
	vpconflictd	512(%edx){1to16}, %zmm6	 # AVX512CD
	vpconflictd	-512(%edx){1to16}, %zmm6	 # AVX512CD Disp8
	vpconflictd	-516(%edx){1to16}, %zmm6	 # AVX512CD

	vpconflictq	%zmm5, %zmm6	 # AVX512CD
	vpconflictq	%zmm5, %zmm6{%k7}	 # AVX512CD
	vpconflictq	%zmm5, %zmm6{%k7}{z}	 # AVX512CD
	vpconflictq	(%ecx), %zmm6	 # AVX512CD
	vpconflictq	-123456(%esp,%esi,8), %zmm6	 # AVX512CD
	vpconflictq	(%eax){1to8}, %zmm6	 # AVX512CD
	vpconflictq	8128(%edx), %zmm6	 # AVX512CD Disp8
	vpconflictq	8192(%edx), %zmm6	 # AVX512CD
	vpconflictq	-8192(%edx), %zmm6	 # AVX512CD Disp8
	vpconflictq	-8256(%edx), %zmm6	 # AVX512CD
	vpconflictq	1016(%edx){1to8}, %zmm6	 # AVX512CD Disp8
	vpconflictq	1024(%edx){1to8}, %zmm6	 # AVX512CD
	vpconflictq	-1024(%edx){1to8}, %zmm6	 # AVX512CD Disp8
	vpconflictq	-1032(%edx){1to8}, %zmm6	 # AVX512CD

	vplzcntd	%zmm5, %zmm6	 # AVX512CD
	vplzcntd	%zmm5, %zmm6{%k7}	 # AVX512CD
	vplzcntd	%zmm5, %zmm6{%k7}{z}	 # AVX512CD
	vplzcntd	(%ecx), %zmm6	 # AVX512CD
	vplzcntd	-123456(%esp,%esi,8), %zmm6	 # AVX512CD
	vplzcntd	(%eax){1to16}, %zmm6	 # AVX512CD
	vplzcntd	8128(%edx), %zmm6	 # AVX512CD Disp8
	vplzcntd	8192(%edx), %zmm6	 # AVX512CD
	vplzcntd	-8192(%edx), %zmm6	 # AVX512CD Disp8
	vplzcntd	-8256(%edx), %zmm6	 # AVX512CD
	vplzcntd	508(%edx){1to16}, %zmm6	 # AVX512CD Disp8
	vplzcntd	512(%edx){1to16}, %zmm6	 # AVX512CD
	vplzcntd	-512(%edx){1to16}, %zmm6	 # AVX512CD Disp8
	vplzcntd	-516(%edx){1to16}, %zmm6	 # AVX512CD

	vplzcntq	%zmm5, %zmm6	 # AVX512CD
	vplzcntq	%zmm5, %zmm6{%k7}	 # AVX512CD
	vplzcntq	%zmm5, %zmm6{%k7}{z}	 # AVX512CD
	vplzcntq	(%ecx), %zmm6	 # AVX512CD
	vplzcntq	-123456(%esp,%esi,8), %zmm6	 # AVX512CD
	vplzcntq	(%eax){1to8}, %zmm6	 # AVX512CD
	vplzcntq	8128(%edx), %zmm6	 # AVX512CD Disp8
	vplzcntq	8192(%edx), %zmm6	 # AVX512CD
	vplzcntq	-8192(%edx), %zmm6	 # AVX512CD Disp8
	vplzcntq	-8256(%edx), %zmm6	 # AVX512CD
	vplzcntq	1016(%edx){1to8}, %zmm6	 # AVX512CD Disp8
	vplzcntq	1024(%edx){1to8}, %zmm6	 # AVX512CD
	vplzcntq	-1024(%edx){1to8}, %zmm6	 # AVX512CD Disp8
	vplzcntq	-1032(%edx){1to8}, %zmm6	 # AVX512CD

	vpbroadcastmw2d	%k6, %zmm6	 # AVX512CD

	vpbroadcastmb2q	%k6, %zmm6	 # AVX512CD

	.intel_syntax noprefix
	vpconflictd	zmm6, zmm5	 # AVX512CD
	vpconflictd	zmm6{k7}, zmm5	 # AVX512CD
	vpconflictd	zmm6{k7}{z}, zmm5	 # AVX512CD
	vpconflictd	zmm6, ZMMWORD PTR [ecx]	 # AVX512CD
	vpconflictd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512CD
	vpconflictd	zmm6, [eax]{1to16}	 # AVX512CD
	vpconflictd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512CD Disp8
	vpconflictd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512CD
	vpconflictd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512CD Disp8
	vpconflictd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512CD
	vpconflictd	zmm6, [edx+508]{1to16}	 # AVX512CD Disp8
	vpconflictd	zmm6, [edx+512]{1to16}	 # AVX512CD
	vpconflictd	zmm6, [edx-512]{1to16}	 # AVX512CD Disp8
	vpconflictd	zmm6, [edx-516]{1to16}	 # AVX512CD

	vpconflictq	zmm6, zmm5	 # AVX512CD
	vpconflictq	zmm6{k7}, zmm5	 # AVX512CD
	vpconflictq	zmm6{k7}{z}, zmm5	 # AVX512CD
	vpconflictq	zmm6, ZMMWORD PTR [ecx]	 # AVX512CD
	vpconflictq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512CD
	vpconflictq	zmm6, [eax]{1to8}	 # AVX512CD
	vpconflictq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512CD Disp8
	vpconflictq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512CD
	vpconflictq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512CD Disp8
	vpconflictq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512CD
	vpconflictq	zmm6, [edx+1016]{1to8}	 # AVX512CD Disp8
	vpconflictq	zmm6, [edx+1024]{1to8}	 # AVX512CD
	vpconflictq	zmm6, [edx-1024]{1to8}	 # AVX512CD Disp8
	vpconflictq	zmm6, [edx-1032]{1to8}	 # AVX512CD

	vplzcntd	zmm6, zmm5	 # AVX512CD
	vplzcntd	zmm6{k7}, zmm5	 # AVX512CD
	vplzcntd	zmm6{k7}{z}, zmm5	 # AVX512CD
	vplzcntd	zmm6, ZMMWORD PTR [ecx]	 # AVX512CD
	vplzcntd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512CD
	vplzcntd	zmm6, [eax]{1to16}	 # AVX512CD
	vplzcntd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512CD Disp8
	vplzcntd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512CD
	vplzcntd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512CD Disp8
	vplzcntd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512CD
	vplzcntd	zmm6, [edx+508]{1to16}	 # AVX512CD Disp8
	vplzcntd	zmm6, [edx+512]{1to16}	 # AVX512CD
	vplzcntd	zmm6, [edx-512]{1to16}	 # AVX512CD Disp8
	vplzcntd	zmm6, [edx-516]{1to16}	 # AVX512CD

	vplzcntq	zmm6, zmm5	 # AVX512CD
	vplzcntq	zmm6{k7}, zmm5	 # AVX512CD
	vplzcntq	zmm6{k7}{z}, zmm5	 # AVX512CD
	vplzcntq	zmm6, ZMMWORD PTR [ecx]	 # AVX512CD
	vplzcntq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512CD
	vplzcntq	zmm6, [eax]{1to8}	 # AVX512CD
	vplzcntq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512CD Disp8
	vplzcntq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512CD
	vplzcntq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512CD Disp8
	vplzcntq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512CD
	vplzcntq	zmm6, [edx+1016]{1to8}	 # AVX512CD Disp8
	vplzcntq	zmm6, [edx+1024]{1to8}	 # AVX512CD
	vplzcntq	zmm6, [edx-1024]{1to8}	 # AVX512CD Disp8
	vplzcntq	zmm6, [edx-1032]{1to8}	 # AVX512CD

	vpbroadcastmw2d	zmm6, k6	 # AVX512CD

	vpbroadcastmb2q	zmm6, k6	 # AVX512CD

