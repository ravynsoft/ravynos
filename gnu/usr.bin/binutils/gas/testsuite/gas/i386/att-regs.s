	.text
	.att_syntax noprefix

	.arch i286
	.code16
	mov	eax, ax			; add	al, (bx,si)
	mov	rax, ax			; add	al, (bx,si)
	mov	axl, ax			; add	al, (bx,si)
	mov	r8b, ax			; add	al, (bx,si)
	mov	r8w, ax			; add	al, (bx,si)
	mov	r8d, ax			; add	al, (bx,si)
	mov	r8, ax			; add	al, (bx,si)
	mov	fs, ax			; add	al, (bx,si)
	mov	st, ax			; add	al, (bx,si)
	mov	cr0, ax			; add	al, (bx,si)
	mov	dr0, ax			; add	al, (bx,si)
	mov	tr0, ax			; add	al, (bx,si)
	mov	mm0, ax			; add	al, (bx,si)
	mov	xmm0, ax		; add	al, (bx,si)
	mov	ymm0, ax		; add	al, (bx,si)

	.arch generic32
	.code32
	mov	rax, eax
	mov	axl, eax
	mov	r8b, eax
	mov	r8w, eax
	mov	r8d, eax
	mov	r8, eax
	mov	st, eax
	mov	cr0, eax
	mov	dr0, eax
	mov	tr0, eax
	mov	mm0, eax
	mov	xmm0, eax
	mov	ymm0, eax

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
	mov	r8b, axl
	mov	r8w, ax
	mov	r8d, eax
	mov	r8, rax
ymm8:
	jmp	ymm8
