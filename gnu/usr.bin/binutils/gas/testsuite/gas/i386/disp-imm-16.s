	.text
	.code16
	mov	-0xffff(%bx), %eax
	vmovaps	-0xffc0(%bx), %zmm0
	add	$-0xffff, %cx

	mov	-0xffff-1(%bx), %eax
	vmovaps	-0xffc0-0x40(%bx), %zmm0
	add	$-0xffff-1, %cx

	mov	-0xffff-2(%bx), %eax
	vmovaps	-0xffc0-0x80(%bx), %zmm0
	add	$-0xffff-2, %cx

	mov	-0x1ffff(%bx), %eax
	vmovaps	-0x1ffc0(%bx), %zmm0
	add	$-0x1ffff, %cx
