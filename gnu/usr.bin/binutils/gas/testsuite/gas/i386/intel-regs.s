	.text
	.intel_syntax noprefix

	mov	eax, tmm1

	.arch i286
	.code16
	mov	ax, eax			; add	[bx+si], al
	mov	ax, rax			; add	[bx+si], al
	mov	ax, axl			; add	[bx+si], al
	mov	ax, r8b			; add	[bx+si], al
	mov	ax, r8w			; add	[bx+si], al
	mov	ax, r8d			; add	[bx+si], al
	mov	ax, r8			; add	[bx+si], al
	mov	ax, fs			; add	[bx+si], al
	mov	ax, st			; add	[bx+si], al
	mov	ax, cr0			; add	[bx+si], al
	mov	ax, dr0			; add	[bx+si], al
	mov	ax, tr0			; add	[bx+si], al
	mov	ax, mm0			; add	[bx+si], al
	mov	ax, xmm0		; add	[bx+si], al
	mov	ax, ymm0		; add	[bx+si], al
	mov	ax, xmm16		; add	[bx+si], al
	mov	ax, zmm0		; add	[bx+si], al

	.arch generic32
	.code32
	mov	eax, rax
	mov	eax, axl
	mov	eax, r8b
	mov	eax, r8w
	mov	eax, r8d
	mov	eax, r8
	mov	eax, st
	mov	eax, cr0
	mov	eax, dr0
	mov	eax, tr0
	mov	eax, mm0
	mov	eax, xmm0
	mov	eax, ymm0
	mov	eax, xmm16
	mov	eax, zmm0

	.arch .387
	ffree	st

	.arch .mmx
	pxor	mm0, mm0

	.arch .sse
	xorps	xmm0, xmm0

	.arch .avx
	vxorps	ymm0, ymm0, ymm0

	.arch generic64
	.code64
	mov	axl, r8b
	mov	ax, r8w
	mov	eax, r8d
	mov	rax, r8
ymm8:
	jmp	ymm8
tmm0:
	jmp	tmm0
