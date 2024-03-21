# Check 32bit AVX512VBMI instructions

	.allow_index_reg
	.text
_start:
	vpermb	%zmm4, %zmm5, %zmm6	 # AVX512VBMI
	vpermb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI
	vpermb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI
	vpermb	(%ecx), %zmm5, %zmm6	 # AVX512VBMI
	vpermb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI
	vpermb	8128(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpermb	8192(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpermb	-8192(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpermb	-8256(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpermi2b	%zmm4, %zmm5, %zmm6	 # AVX512VBMI
	vpermi2b	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI
	vpermi2b	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI
	vpermi2b	(%ecx), %zmm5, %zmm6	 # AVX512VBMI
	vpermi2b	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI
	vpermi2b	8128(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpermi2b	8192(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpermi2b	-8192(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpermi2b	-8256(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpermt2b	%zmm4, %zmm5, %zmm6	 # AVX512VBMI
	vpermt2b	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI
	vpermt2b	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI
	vpermt2b	(%ecx), %zmm5, %zmm6	 # AVX512VBMI
	vpermt2b	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI
	vpermt2b	8128(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpermt2b	8192(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpermt2b	-8192(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpermt2b	-8256(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	%zmm4, %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI
	vpmultishiftqb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI
	vpmultishiftqb	(%ecx), %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	(%eax){1to8}, %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	8128(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpmultishiftqb	8192(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	-8192(%edx), %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpmultishiftqb	-8256(%edx), %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpmultishiftqb	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI
	vpmultishiftqb	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI Disp8
	vpmultishiftqb	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI

	.intel_syntax noprefix
	vpermb	zmm6, zmm5, zmm4	 # AVX512VBMI
	vpermb	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI
	vpermb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI
	vpermb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512VBMI
	vpermb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI
	vpermb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512VBMI Disp8
	vpermb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512VBMI
	vpermb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512VBMI Disp8
	vpermb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512VBMI
	vpermi2b	zmm6, zmm5, zmm4	 # AVX512VBMI
	vpermi2b	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI
	vpermi2b	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI
	vpermi2b	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512VBMI
	vpermi2b	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI
	vpermi2b	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512VBMI Disp8
	vpermi2b	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512VBMI
	vpermi2b	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512VBMI Disp8
	vpermi2b	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512VBMI
	vpermt2b	zmm6, zmm5, zmm4	 # AVX512VBMI
	vpermt2b	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI
	vpermt2b	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI
	vpermt2b	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512VBMI
	vpermt2b	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI
	vpermt2b	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512VBMI Disp8
	vpermt2b	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512VBMI
	vpermt2b	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512VBMI Disp8
	vpermt2b	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, zmm4	 # AVX512VBMI
	vpmultishiftqb	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI
	vpmultishiftqb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, [eax]{1to8}	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, [edx+1016]{1to8}	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm6, zmm5, [edx+1024]{1to8}	 # AVX512VBMI
	vpmultishiftqb	zmm6, zmm5, [edx-1024]{1to8}	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm6, zmm5, [edx-1032]{1to8}	 # AVX512VBMI
