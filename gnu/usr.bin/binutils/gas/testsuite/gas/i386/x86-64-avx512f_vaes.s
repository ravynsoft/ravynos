# Check 64bit AVX512F,VAES instructions

	.allow_index_reg
	.text
_start:
	vaesdec	%zmm28, %zmm29, %zmm30	 # AVX512F,VAES
	vaesdec	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,VAES
	vaesdec	8128(%rdx), %zmm5, %zmm6	 # AVX512F,VAES Disp8
	vaesdeclast	%zmm28, %zmm29, %zmm30	 # AVX512F,VAES
	vaesdeclast	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,VAES
	vaesdeclast	8128(%rdx), %zmm5, %zmm6	 # AVX512F,VAES Disp8
	vaesenc	%zmm28, %zmm29, %zmm30	 # AVX512F,VAES
	vaesenc	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,VAES
	vaesenc	8128(%rdx), %zmm5, %zmm6	 # AVX512F,VAES Disp8
	vaesenclast	%zmm28, %zmm29, %zmm30	 # AVX512F,VAES
	vaesenclast	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,VAES
	vaesenclast	8128(%rdx), %zmm5, %zmm6	 # AVX512F,VAES Disp8

	.intel_syntax noprefix
	vaesdec	zmm30, zmm29, zmm28	 # AVX512F,VAES
	vaesdec	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F,VAES
	vaesdec	zmm6, zmm5, ZMMWORD PTR [rdx+8128]	 # AVX512F,VAES Disp8
	vaesdeclast	zmm30, zmm29, zmm28	 # AVX512F,VAES
	vaesdeclast	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F,VAES
	vaesdeclast	zmm6, zmm5, ZMMWORD PTR [rdx+8128]	 # AVX512F,VAES Disp8
	vaesenc	zmm30, zmm29, zmm28	 # AVX512F,VAES
	vaesenc	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F,VAES
	vaesenc	zmm6, zmm5, ZMMWORD PTR [rdx+8128]	 # AVX512F,VAES Disp8
	vaesenclast	zmm30, zmm29, zmm28	 # AVX512F,VAES
	vaesenclast	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F,VAES
	vaesenclast	zmm6, zmm5, ZMMWORD PTR [rdx+8128]	 # AVX512F,VAES Disp8
