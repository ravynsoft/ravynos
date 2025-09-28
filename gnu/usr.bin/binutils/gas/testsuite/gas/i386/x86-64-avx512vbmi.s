# Check 64bit AVX512VBMI instructions

	.allow_index_reg
	.text
_start:
	vpermb	%zmm28, %zmm29, %zmm30	 # AVX512VBMI
	vpermb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI
	vpermb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI
	vpermb	(%rcx), %zmm29, %zmm30	 # AVX512VBMI
	vpermb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI
	vpermb	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpermb	8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpermb	-8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpermb	-8256(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpermi2b	%zmm28, %zmm29, %zmm30	 # AVX512VBMI
	vpermi2b	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI
	vpermi2b	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI
	vpermi2b	(%rcx), %zmm29, %zmm30	 # AVX512VBMI
	vpermi2b	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI
	vpermi2b	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpermi2b	8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpermi2b	-8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpermi2b	-8256(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpermt2b	%zmm28, %zmm29, %zmm30	 # AVX512VBMI
	vpermt2b	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI
	vpermt2b	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI
	vpermt2b	(%rcx), %zmm29, %zmm30	 # AVX512VBMI
	vpermt2b	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI
	vpermt2b	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpermt2b	8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpermt2b	-8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpermt2b	-8256(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	%zmm28, %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI
	vpmultishiftqb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI
	vpmultishiftqb	(%rcx), %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpmultishiftqb	8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	-8192(%rdx), %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpmultishiftqb	-8256(%rdx), %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpmultishiftqb	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512VBMI
	vpmultishiftqb	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512VBMI Disp8
	vpmultishiftqb	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512VBMI

	.intel_syntax noprefix
	vpermb	zmm30, zmm29, zmm28	 # AVX512VBMI
	vpermb	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI
	vpermb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI
	vpermb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512VBMI
	vpermb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI
	vpermb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI Disp8
	vpermb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512VBMI
	vpermb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512VBMI Disp8
	vpermb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512VBMI
	vpermi2b	zmm30, zmm29, zmm28	 # AVX512VBMI
	vpermi2b	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI
	vpermi2b	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI
	vpermi2b	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512VBMI
	vpermi2b	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI
	vpermi2b	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI Disp8
	vpermi2b	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512VBMI
	vpermi2b	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512VBMI Disp8
	vpermi2b	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512VBMI
	vpermt2b	zmm30, zmm29, zmm28	 # AVX512VBMI
	vpermt2b	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI
	vpermt2b	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI
	vpermt2b	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512VBMI
	vpermt2b	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI
	vpermt2b	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI Disp8
	vpermt2b	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512VBMI
	vpermt2b	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512VBMI Disp8
	vpermt2b	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, zmm28	 # AVX512VBMI
	vpmultishiftqb	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI
	vpmultishiftqb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, [rcx]{1to8}	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, [rdx+1016]{1to8}	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm30, zmm29, [rdx+1024]{1to8}	 # AVX512VBMI
	vpmultishiftqb	zmm30, zmm29, [rdx-1024]{1to8}	 # AVX512VBMI Disp8
	vpmultishiftqb	zmm30, zmm29, [rdx-1032]{1to8}	 # AVX512VBMI
