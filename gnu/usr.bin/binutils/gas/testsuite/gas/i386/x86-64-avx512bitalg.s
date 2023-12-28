# Check 64bit AVX512BITALG instructions

	.allow_index_reg
	.text
_start:
	vpshufbitqmb	%zmm28, %zmm29, %k5	 # AVX512BITALG
	vpshufbitqmb	%zmm28, %zmm29, %k5{%k7}	 # AVX512BITALG
	vpshufbitqmb	0x123(%rax,%r14,8), %zmm29, %k5	 # AVX512BITALG
	vpshufbitqmb	8128(%rdx), %zmm29, %k5	 # AVX512BITALG Disp8

	vpopcntb	%zmm29, %zmm30	 # AVX512BITALG
	vpopcntb	%zmm29, %zmm30{%k7}	 # AVX512BITALG
	vpopcntb	%zmm29, %zmm30{%k7}{z}	 # AVX512BITALG
	vpopcntb	0x123(%rax,%r14,8), %zmm30	 # AVX512BITALG
	vpopcntb	8128(%rdx), %zmm30	 # AVX512BITALG Disp8

	vpopcntw	%zmm29, %zmm30	 # AVX512BITALG
	vpopcntw	%zmm29, %zmm30{%k7}	 # AVX512BITALG
	vpopcntw	%zmm29, %zmm30{%k7}{z}	 # AVX512BITALG
	vpopcntw	0x123(%rax,%r14,8), %zmm30	 # AVX512BITALG
	vpopcntw	8128(%rdx), %zmm30	 # AVX512BITALG Disp8

	vpopcntd	%zmm29, %zmm30	 # AVX512BITALG
	vpopcntd	%zmm29, %zmm30{%k7}	 # AVX512BITALG
	vpopcntd	%zmm29, %zmm30{%k7}{z}	 # AVX512BITALG
	vpopcntd	0x123(%rax,%r14,8), %zmm30	 # AVX512BITALG
	vpopcntd	8128(%rdx), %zmm30	 # AVX512BITALG Disp8
	vpopcntd	508(%rdx){1to16}, %zmm30	 # AVX512BITALG Disp8

	vpopcntq	%zmm29, %zmm30	 # AVX512BITALG
	vpopcntq	%zmm29, %zmm30{%k7}	 # AVX512BITALG
	vpopcntq	%zmm29, %zmm30{%k7}{z}	 # AVX512BITALG
	vpopcntq	0x123(%rax,%r14,8), %zmm30	 # AVX512BITALG
	vpopcntq	8128(%rdx), %zmm30	 # AVX512BITALG Disp8
	vpopcntq	1016(%rdx){1to8}, %zmm30	 # AVX512BITALG Disp8

	.intel_syntax noprefix
	vpshufbitqmb	k5, zmm29, zmm28	 # AVX512BITALG
	vpshufbitqmb	k5{k7}, zmm29, zmm28	 # AVX512BITALG
	vpshufbitqmb	k5, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BITALG
	vpshufbitqmb	k5, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BITALG Disp8

	vpopcntb	zmm30, zmm29	 # AVX512BITALG
	vpopcntb	zmm30{k7}, zmm29	 # AVX512BITALG
	vpopcntb	zmm30{k7}{z}, zmm29	 # AVX512BITALG
	vpopcntb	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BITALG
	vpopcntb	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BITALG Disp8

	vpopcntw	zmm30, zmm29	 # AVX512BITALG
	vpopcntw	zmm30{k7}, zmm29	 # AVX512BITALG
	vpopcntw	zmm30{k7}{z}, zmm29	 # AVX512BITALG
	vpopcntw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BITALG
	vpopcntw	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BITALG Disp8

	vpopcntd	zmm30, zmm29	 # AVX512BITALG
	vpopcntd	zmm30{k7}, zmm29	 # AVX512BITALG
	vpopcntd	zmm30{k7}{z}, zmm29	 # AVX512BITALG
	vpopcntd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BITALG
	vpopcntd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BITALG Disp8
	vpopcntd	zmm30, [rdx+508]{1to16}	 # AVX512BITALG Disp8

	vpopcntq	zmm30, zmm29	 # AVX512BITALG
	vpopcntq	zmm30{k7}, zmm29	 # AVX512BITALG
	vpopcntq	zmm30{k7}{z}, zmm29	 # AVX512BITALG
	vpopcntq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BITALG
	vpopcntq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BITALG Disp8
	vpopcntq	zmm30, [rdx+1016]{1to8}	 # AVX512BITALG Disp8
