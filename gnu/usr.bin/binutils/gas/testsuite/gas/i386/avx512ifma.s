# Check 32bit AVX512IFMA instructions

	.allow_index_reg
	.text
_start:
	vpmadd52luq	%zmm4, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512IFMA
	vpmadd52luq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512IFMA
	vpmadd52luq	(%ecx), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	8128(%edx), %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52luq	8192(%edx), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	-8192(%edx), %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52luq	-8256(%edx), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52luq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52luq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52luq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	%zmm4, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512IFMA
	vpmadd52huq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512IFMA
	vpmadd52huq	(%ecx), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	8128(%edx), %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52huq	8192(%edx), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	-8192(%edx), %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52huq	-8256(%edx), %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52huq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA
	vpmadd52huq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA Disp8
	vpmadd52huq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512IFMA

	.intel_syntax noprefix
	vpmadd52luq	zmm6, zmm5, zmm4	 # AVX512IFMA
	vpmadd52luq	zmm6{k7}, zmm5, zmm4	 # AVX512IFMA
	vpmadd52luq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, [eax]{1to8}	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512IFMA Disp8
	vpmadd52luq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512IFMA Disp8
	vpmadd52luq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, [edx+1016]{1to8}	 # AVX512IFMA Disp8
	vpmadd52luq	zmm6, zmm5, [edx+1024]{1to8}	 # AVX512IFMA
	vpmadd52luq	zmm6, zmm5, [edx-1024]{1to8}	 # AVX512IFMA Disp8
	vpmadd52luq	zmm6, zmm5, [edx-1032]{1to8}	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, zmm4	 # AVX512IFMA
	vpmadd52huq	zmm6{k7}, zmm5, zmm4	 # AVX512IFMA
	vpmadd52huq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, [eax]{1to8}	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512IFMA Disp8
	vpmadd52huq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512IFMA Disp8
	vpmadd52huq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, [edx+1016]{1to8}	 # AVX512IFMA Disp8
	vpmadd52huq	zmm6, zmm5, [edx+1024]{1to8}	 # AVX512IFMA
	vpmadd52huq	zmm6, zmm5, [edx-1024]{1to8}	 # AVX512IFMA Disp8
	vpmadd52huq	zmm6, zmm5, [edx-1032]{1to8}	 # AVX512IFMA
