# Check 32bit AVX512BITALG instructions

	.allow_index_reg
	.text
_start:
	vpshufbitqmb	%zmm4, %zmm5, %k5	 # AVX512BITALG
	vpshufbitqmb	%zmm4, %zmm5, %k5{%k7}	 # AVX512BITALG
	vpshufbitqmb	-123456(%esp,%esi,8), %zmm5, %k5	 # AVX512BITALG
	vpshufbitqmb	8128(%edx), %zmm5, %k5	 # AVX512BITALG Disp8

	vpopcntb	%zmm5, %zmm6	 # AVX512BITALG
	vpopcntb	%zmm5, %zmm6{%k7}	 # AVX512BITALG
	vpopcntb	%zmm5, %zmm6{%k7}{z}	 # AVX512BITALG
	vpopcntb	-123456(%esp,%esi,8), %zmm6	 # AVX512BITALG
	vpopcntb	8128(%edx), %zmm6	 # AVX512BITALG Disp8

	vpopcntw	%zmm5, %zmm6	 # AVX512BITALG
	vpopcntw	%zmm5, %zmm6{%k7}	 # AVX512BITALG
	vpopcntw	%zmm5, %zmm6{%k7}{z}	 # AVX512BITALG
	vpopcntw	-123456(%esp,%esi,8), %zmm6	 # AVX512BITALG
	vpopcntw	8128(%edx), %zmm6	 # AVX512BITALG Disp8
	vpopcntd	%zmm5, %zmm6	 # AVX512BITALG

	vpopcntd	%zmm5, %zmm6{%k7}	 # AVX512BITALG
	vpopcntd	%zmm5, %zmm6{%k7}{z}	 # AVX512BITALG
	vpopcntd	-123456(%esp,%esi,8), %zmm6	 # AVX512BITALG
	vpopcntd	8128(%edx), %zmm6	 # AVX512BITALG Disp8
	vpopcntd	508(%edx){1to16}, %zmm6	 # AVX512BITALG Disp8
	vpopcntq	%zmm5, %zmm6	 # AVX512BITALG

	vpopcntq	%zmm5, %zmm6{%k7}	 # AVX512BITALG
	vpopcntq	%zmm5, %zmm6{%k7}{z}	 # AVX512BITALG
	vpopcntq	-123456(%esp,%esi,8), %zmm6	 # AVX512BITALG
	vpopcntq	8128(%edx), %zmm6	 # AVX512BITALG Disp8
	vpopcntq	1016(%edx){1to8}, %zmm6	 # AVX512BITALG Disp8

	.intel_syntax noprefix
	vpshufbitqmb	k5, zmm5, zmm4	 # AVX512BITALG
	vpshufbitqmb	k5{k7}, zmm5, zmm4	 # AVX512BITALG
	vpshufbitqmb	k5, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BITALG
	vpshufbitqmb	k5, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BITALG Disp8
	vpopcntb	zmm6, zmm5	 # AVX512BITALG

	vpopcntb	zmm6{k7}, zmm5	 # AVX512BITALG
	vpopcntb	zmm6{k7}{z}, zmm5	 # AVX512BITALG
	vpopcntb	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BITALG
	vpopcntb	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BITALG Disp8
	vpopcntw	zmm6, zmm5	 # AVX512BITALG

	vpopcntw	zmm6{k7}, zmm5	 # AVX512BITALG
	vpopcntw	zmm6{k7}{z}, zmm5	 # AVX512BITALG
	vpopcntw	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BITALG
	vpopcntw	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BITALG Disp8
	vpopcntd	zmm6, zmm5	 # AVX512BITALG

	vpopcntd	zmm6{k7}, zmm5	 # AVX512BITALG
	vpopcntd	zmm6{k7}{z}, zmm5	 # AVX512BITALG
	vpopcntd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BITALG
	vpopcntd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BITALG Disp8
	vpopcntd	zmm6, [edx+508]{1to16}	 # AVX512BITALG Disp8
	vpopcntq	zmm6, zmm5	 # AVX512BITALG

	vpopcntq	zmm6{k7}, zmm5	 # AVX512BITALG
	vpopcntq	zmm6{k7}{z}, zmm5	 # AVX512BITALG
	vpopcntq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BITALG
	vpopcntq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BITALG Disp8
	vpopcntq	zmm6, [edx+1016]{1to8}	 # AVX512BITALG Disp8
