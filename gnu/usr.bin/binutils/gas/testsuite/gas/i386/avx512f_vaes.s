# Check 32bit AVX512F,VAES instructions

	.allow_index_reg
	.text
_start:
	vaesdec	%zmm4, %zmm5, %zmm6	 # AVX512F,VAES
	vaesdec	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,VAES
	vaesdec	8128(%edx), %zmm5, %zmm6	 # AVX512F,VAES Disp8

	vaesdeclast	%zmm4, %zmm5, %zmm6	 # AVX512F,VAES
	vaesdeclast	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,VAES
	vaesdeclast	8128(%edx), %zmm5, %zmm6	 # AVX512F,VAES Disp8

	vaesenc	%zmm4, %zmm5, %zmm6	 # AVX512F,VAES
	vaesenc	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,VAES
	vaesenc	8128(%edx), %zmm5, %zmm6	 # AVX512F,VAES Disp8

	vaesenclast	%zmm4, %zmm5, %zmm6	 # AVX512F,VAES
	vaesenclast	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,VAES
	vaesenclast	8128(%edx), %zmm5, %zmm6	 # AVX512F,VAES Disp8

	.intel_syntax noprefix
	vaesdec	zmm6, zmm5, zmm4	 # AVX512F,VAES
	vaesdec	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F,VAES
	vaesdec	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F,VAES Disp8

	vaesdeclast	zmm6, zmm5, zmm4	 # AVX512F,VAES
	vaesdeclast	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F,VAES
	vaesdeclast	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F,VAES Disp8

	vaesenc	zmm6, zmm5, zmm4	 # AVX512F,VAES
	vaesenc	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F,VAES
	vaesenc	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F,VAES Disp8

	vaesenclast	zmm6, zmm5, zmm4	 # AVX512F,VAES
	vaesenclast	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F,VAES
	vaesenclast	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F,VAES Disp8
