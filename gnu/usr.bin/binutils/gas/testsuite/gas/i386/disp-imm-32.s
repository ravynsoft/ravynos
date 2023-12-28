	.text
disp_imm:
	mov	-0xffffffff(%eax), %eax
	vmovaps	-0xffffffc0(%eax), %zmm0
	add	$-0xffffffff, %ecx

	mov	-0xffffffff-1(%eax), %eax
	vmovaps	-0xffffffc0-0x40(%eax), %zmm0
	add	$-0xffffffff-1, %ecx

	mov	-0xffffffff-2(%eax), %eax
	vmovaps	-0xffffffc0-0x80(%eax), %zmm0
	add	$-0xffffffff-2, %ecx

	mov	-0x1ffffffff(%eax), %eax
	vmovaps	-0x1ffffffc0(%eax), %zmm0
	add	$-0x1ffffffff, %ecx
