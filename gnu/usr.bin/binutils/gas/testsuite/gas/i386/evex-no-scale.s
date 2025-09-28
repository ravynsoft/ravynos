	.allow_index_reg
	.struct
	inc	%eax
.equiv is_64bit, . > 1

	.text
disp:
.if is_64bit
	vmovaps	-1024(%rip), %zmm0
	vmovaps	64(,%rax), %zmm0
	vmovaps	64(,%riz), %zmm0
.endif
	vmovaps	64(,%eax), %zmm0
	vmovaps	64(,%eiz), %zmm0
	vmovaps	64, %zmm0
.if !is_64bit
	addr16 vmovaps 64, %zmm0
.endif
