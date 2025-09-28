# IAMCU instructions
	.text

	movsx	%al, %si
	movsx	%al, %esi
	movsx	%ax, %esi
	movsx	(%eax), %dx
	movsxb	(%eax), %dx
	movsxb	(%eax), %edx
	movsxw	(%eax), %edx
	movsbl	(%eax), %edx
	movsbw	(%eax), %dx
	movswl	(%eax), %edx

	movzx	%al, %si
	movzx	%al, %esi
	movzx	%ax, %esi
	movzx	(%eax), %dx
	movzxb	(%eax), %dx
	movzxb	(%eax), %edx
	movzxw	(%eax), %edx
	movzb	(%eax), %edx
	movzb	(%eax), %dx
	movzbl	(%eax), %edx
	movzbw	(%eax), %dx
	movzwl	(%eax), %edx

	.intel_syntax noprefix

	movsx	si,al
	movsx	esi,al
	movsx	esi,ax
	movsx	edx,BYTE PTR [eax]
	movsx	dx,BYTE PTR [eax]
	movsx	edx,WORD PTR [eax]

	movzx	si,al
	movzx	esi,al
	movzx	esi,ax
	movzx	edx,BYTE PTR [eax]
	movzx	dx,BYTE PTR [eax]
	movzx	edx,WORD PTR [eax]

	movsx ax, byte ptr [eax]
	movsx eax, byte ptr [eax]
	movsx eax, word ptr [eax]
	movzx ax, byte ptr [eax]
	movzx eax, byte ptr [eax]
	movzx eax, word ptr [eax]
