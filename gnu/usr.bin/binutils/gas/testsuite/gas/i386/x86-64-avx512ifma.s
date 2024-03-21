# Check 64bit AVX512IFMA instructions

	.allow_index_reg
	.text
_start:
	vpmadd52luq	%zmm28, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512IFMA
	vpmadd52luq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512IFMA
	vpmadd52luq	(%rcx), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	8128(%rdx), %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52luq	8192(%rdx), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	-8192(%rdx), %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52luq	-8256(%rdx), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52luq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52luq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52luq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	%zmm28, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512IFMA
	vpmadd52huq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512IFMA
	vpmadd52huq	(%rcx), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	8128(%rdx), %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52huq	8192(%rdx), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	-8192(%rdx), %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52huq	-8256(%rdx), %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52huq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA
	vpmadd52huq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA Disp8
	vpmadd52huq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512IFMA

	.intel_syntax noprefix
	vpmadd52luq	zmm30, zmm29, zmm28	 # AVX512IFMA
	vpmadd52luq	zmm30{k7}, zmm29, zmm28	 # AVX512IFMA
	vpmadd52luq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, [rcx]{1to8}	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512IFMA Disp8
	vpmadd52luq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512IFMA Disp8
	vpmadd52luq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, [rdx+1016]{1to8}	 # AVX512IFMA Disp8
	vpmadd52luq	zmm30, zmm29, [rdx+1024]{1to8}	 # AVX512IFMA
	vpmadd52luq	zmm30, zmm29, [rdx-1024]{1to8}	 # AVX512IFMA Disp8
	vpmadd52luq	zmm30, zmm29, [rdx-1032]{1to8}	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, zmm28	 # AVX512IFMA
	vpmadd52huq	zmm30{k7}, zmm29, zmm28	 # AVX512IFMA
	vpmadd52huq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, [rcx]{1to8}	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512IFMA Disp8
	vpmadd52huq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512IFMA Disp8
	vpmadd52huq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, [rdx+1016]{1to8}	 # AVX512IFMA Disp8
	vpmadd52huq	zmm30, zmm29, [rdx+1024]{1to8}	 # AVX512IFMA
	vpmadd52huq	zmm30, zmm29, [rdx-1024]{1to8}	 # AVX512IFMA Disp8
	vpmadd52huq	zmm30, zmm29, [rdx-1032]{1to8}	 # AVX512IFMA
