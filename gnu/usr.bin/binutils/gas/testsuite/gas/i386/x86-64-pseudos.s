# Check 64bit instructions with pseudo prefixes for encoding

	.text
_start:
	{vex3} vmovaps %xmm7,%xmm2
	{vex3} {load} vmovaps %xmm7,%xmm2
	{vex3} {store} vmovaps %xmm7,%xmm2
	vmovaps %xmm7,%xmm2
	{vex} vmovaps %xmm7,%xmm2
	{vex} {load} vmovaps %xmm7,%xmm2
	{vex} {store} vmovaps %xmm7,%xmm2
	{vex3} vmovaps (%rax),%xmm2
	vmovaps (%rax),%xmm2
	{vex2} vmovaps (%rax),%xmm2
	{evex} vmovaps (%rax),%xmm2
	{disp32} vmovaps (%rax),%xmm2
	{evex} {disp8} vmovaps (%rax),%xmm2
	{evex} {disp32} vmovaps (%rax),%xmm2

	{vex} {disp8} vmovaps 128(%rax),%xmm2
	{vex} {disp32} vmovaps 128(%rax),%xmm2
	{evex} {disp8} vmovaps 128(%rax),%xmm2
	{evex} {disp32} vmovaps 128(%rax),%xmm2

	mov %rcx, %rax
	{load} mov %rcx, %rax
	{store} mov %rcx, %rax
	adc %ecx, %eax
	{load} adc %ecx, %eax
	{store} adc %ecx, %eax
	add %ecx, %eax
	{load} add %ecx, %eax
	{store} add %ecx, %eax
	and %ecx, %eax
	{load} and %ecx, %eax
	{store} and %ecx, %eax
	cmp %ecx, %eax
	{load} cmp %ecx, %eax
	{store} cmp %ecx, %eax
	or %ecx, %eax
	{load} or %ecx, %eax
	{store} or %ecx, %eax
	sbb %ecx, %eax
	{load} sbb %ecx, %eax
	{store} sbb %ecx, %eax
	sub %ecx, %eax
	{load} sub %ecx, %eax
	{store} sub %ecx, %eax
	xor %ecx, %eax
	{load} xor %ecx, %eax
	{store} xor %ecx, %eax

	{load} mov 0x12345678, %eax
	{load} mov %eax, 0x12345678
	{store} mov 0x12345678, %eax
	{store} mov %eax, 0x12345678
	{load} mov 0x123456789abcdef0, %eax
	{load} mov %eax, 0x123456789abcdef0
	{store} mov 0x123456789abcdef0, %eax
	{store} mov %eax, 0x123456789abcdef0
	{load} movabs 0x123456789abcdef0, %eax
	{load} movabs %eax, 0x123456789abcdef0
	{store} movabs 0x123456789abcdef0, %eax
	{store} movabs %eax, 0x123456789abcdef0
	{load} mov %eax, (%rdi)
	{load} mov (%rdi), %eax
	{store} mov %eax, (%rdi)
	{store} mov (%rdi), %eax
	{load} mov %es, %edi
	{load} mov %eax, %gs
	{store} mov %es, %edi
	{store} mov %eax, %gs
	{load} mov %cr0, %rdi
	{load} mov %rax, %cr7
	{store} mov %cr0, %rdi
	{store} mov %rax, %cr7
	{load} mov %dr0, %rdi
	{load} mov %rax, %dr7
	{store} mov %dr0, %rdi
	{store} mov %rax, %dr7
	{load} kmovb %k0, %edi
	{load} kmovb %eax, %k7
	{store} kmovb %k0, %edi
	{store} kmovb %eax, %k7
	{load} kmovd %k0, %edi
	{load} kmovd %eax, %k7
	{store} kmovd %k0, %edi
	{store} kmovd %eax, %k7
	{load} kmovq %k0, %rdi
	{load} kmovq %rax, %k7
	{store} kmovq %k0, %rdi
	{store} kmovq %rax, %k7
	{load} kmovw %k0, %edi
	{load} kmovw %eax, %k7
	{store} kmovw %k0, %edi
	{store} kmovw %eax, %k7
	{load} kmovb %k0, %k7
	{store} kmovb %k0, %k7
	{load} kmovd %k0, %k7
	{store} kmovd %k0, %k7
	{load} kmovq %k0, %k7
	{store} kmovq %k0, %k7
	{load} kmovw %k0, %k7
	{store} kmovw %k0, %k7
	{load} adc %eax, (%rdi)
	{load} adc (%rdi), %eax
	{store} adc %eax, (%rdi)
	{store} adc (%rdi), %eax
	{load} add %eax, (%rdi)
	{load} add (%rdi), %eax
	{store} add %eax, (%rdi)
	{store} add (%rdi), %eax
	{load} and %eax, (%rdi)
	{load} and (%rdi), %eax
	{store} and %eax, (%rdi)
	{store} and (%rdi), %eax
	{load} cmp %eax, (%rdi)
	{load} cmp (%rdi), %eax
	{store} cmp %eax, (%rdi)
	{store} cmp (%rdi), %eax
	{load} or %eax, (%rdi)
	{load} or (%rdi), %eax
	{store} or %eax, (%rdi)
	{store} or (%rdi), %eax
	{load} sbb %eax, (%rdi)
	{load} sbb (%rdi), %eax
	{store} sbb %eax, (%rdi)
	{store} sbb (%rdi), %eax
	{load} sub %eax, (%rdi)
	{load} sub (%rdi), %eax
	{store} sub %eax, (%rdi)
	{store} sub (%rdi), %eax
	{load} xor %eax, (%rdi)
	{load} xor (%rdi), %eax
	{store} xor %eax, (%rdi)
	{store} xor (%rdi), %eax

	.irp m, mov, adc, add, and, cmp, or, sbb, sub, test, xor
	\m	$0x12, %al
	\m	$0x345, %eax
	{load} \m $0x12, %al		# bogus for MOV
	{load} \m $0x345, %eax		# bogus for MOV
	{store} \m $0x12, %al
	{store} \m $0x345, %eax
	.endr

	# There should be no effect of the pseudo-prefixes on any of these.
	mov	$0x123456789, %rcx
	{load} mov $0x123456789, %rcx
	{store} mov $0x123456789, %rcx
	movabs	$0x12345678, %rcx
	{load} movabs $0x12345678, %rcx
	{store} movabs $0x12345678, %rcx

	.irp m, push, pop, bswap
	\m	%rcx
	{load} \m %rcx			# bogus for POP
	{store} \m %rcx			# bogus for PUSH
	.endr

	xchg	%ecx, %esi
	xchg	%esi, %ecx
	{load} xchg %ecx, %esi
	{store} xchg %ecx, %esi

	xchg	%eax, %esi
	{load} xchg %eax, %esi
	{store} xchg %eax, %esi

	xchg	%ecx, %eax
	{load} xchg %ecx, %eax
	{store} xchg %ecx, %eax

	fadd %st, %st
	{load} fadd %st, %st
	{store} fadd %st, %st
	fdiv %st, %st
	{load} fdiv %st, %st
	{store} fdiv %st, %st
	fdivr %st, %st
	{load} fdivr %st, %st
	{store} fdivr %st, %st
	fmul %st, %st
	{load} fmul %st, %st
	{store} fmul %st, %st
	fsub %st, %st
	{load} fsub %st, %st
	{store} fsub %st, %st
	fsubr %st, %st
	{load} fsubr %st, %st
	{store} fsubr %st, %st

	movq %mm0, %mm7
	{load} movq %mm0, %mm7
	{store} movq %mm0, %mm7

	movaps %xmm0, %xmm7
	{load} movaps %xmm0, %xmm7
	{store} movaps %xmm0, %xmm7
	movups %xmm0, %xmm7
	{load} movups %xmm0, %xmm7
	{store} movups %xmm0, %xmm7
	movss %xmm0, %xmm7
	{load} movss %xmm0, %xmm7
	{store} movss %xmm0, %xmm7
	movapd %xmm0, %xmm7
	{load} movapd %xmm0, %xmm7
	{store} movapd %xmm0, %xmm7
	movupd %xmm0, %xmm7
	{load} movupd %xmm0, %xmm7
	{store} movupd %xmm0, %xmm7
	movsd %xmm0, %xmm7
	{load} movsd %xmm0, %xmm7
	{store} movsd %xmm0, %xmm7
	movdqa %xmm0, %xmm7
	{load} movdqa %xmm0, %xmm7
	{store} movdqa %xmm0, %xmm7
	movdqu %xmm0, %xmm7
	{load} movdqu %xmm0, %xmm7
	{store} movdqu %xmm0, %xmm7
	movq %xmm0, %xmm7
	{load} movq %xmm0, %xmm7
	{store} movq %xmm0, %xmm7
	vmovaps %xmm0, %xmm7
	{load} vmovaps %xmm0, %xmm7
	{store} vmovaps %xmm0, %xmm7
	vmovaps %zmm0, %zmm7
	{load} vmovaps %zmm0, %zmm7
	{store} vmovaps %zmm0, %zmm7
	vmovaps %xmm0, %xmm7{%k7}
	{load} vmovaps %xmm0, %xmm7{%k7}
	{store} vmovaps %xmm0, %xmm7{%k7}
	vmovups %zmm0, %zmm7
	{load} vmovups %zmm0, %zmm7
	{store} vmovups %zmm0, %zmm7
	vmovups %xmm0, %xmm7
	{load} vmovups %xmm0, %xmm7
	{store} vmovups %xmm0, %xmm7
	vmovups %xmm0, %xmm7{%k7}
	{load} vmovups %xmm0, %xmm7{%k7}
	{store} vmovups %xmm0, %xmm7{%k7}
	vmovss %xmm0, %xmm1, %xmm7
	{load} vmovss %xmm0, %xmm1, %xmm7
	{store} vmovss %xmm0, %xmm1, %xmm7
	vmovss %xmm0, %xmm1, %xmm7{%k7}
	{load} vmovss %xmm0, %xmm1, %xmm7{%k7}
	{store} vmovss %xmm0, %xmm1, %xmm7{%k7}
	vmovapd %xmm0, %xmm7
	{load} vmovapd %xmm0, %xmm7
	{store} vmovapd %xmm0, %xmm7
	vmovapd %zmm0, %zmm7
	{load} vmovapd %zmm0, %zmm7
	{store} vmovapd %zmm0, %zmm7
	vmovapd %xmm0, %xmm7{%k7}
	{load} vmovapd %xmm0, %xmm7{%k7}
	{store} vmovapd %xmm0, %xmm7{%k7}
	vmovupd %xmm0, %xmm7
	{load} vmovupd %xmm0, %xmm7
	{store} vmovupd %xmm0, %xmm7
	vmovupd %zmm0, %zmm7
	{load} vmovupd %zmm0, %zmm7
	{store} vmovupd %zmm0, %zmm7
	vmovupd %xmm0, %xmm7{%k7}
	{load} vmovupd %xmm0, %xmm7{%k7}
	{store} vmovupd %xmm0, %xmm7{%k7}
	vmovsd %xmm0, %xmm1, %xmm7
	{load} vmovsd %xmm0, %xmm1, %xmm7
	{store} vmovsd %xmm0, %xmm1, %xmm7
	vmovsd %xmm0, %xmm1, %xmm7{%k7}
	{load} vmovsd %xmm0, %xmm1, %xmm7{%k7}
	{store} vmovsd %xmm0, %xmm1, %xmm7{%k7}
	vmovdqa %xmm0, %xmm7
	{load} vmovdqa %xmm0, %xmm7
	{store} vmovdqa %xmm0, %xmm7
	vmovdqa32 %zmm0, %zmm7
	{load} vmovdqa32 %zmm0, %zmm7
	{store} vmovdqa32 %zmm0, %zmm7
	vmovdqa32 %xmm0, %xmm7
	{load} vmovdqa32 %xmm0, %xmm7
	{store} vmovdqa32 %xmm0, %xmm7
	vmovdqa64 %zmm0, %zmm7
	{load} vmovdqa64 %zmm0, %zmm7
	{store} vmovdqa64 %zmm0, %zmm7
	vmovdqa64 %xmm0, %xmm7
	{load} vmovdqa64 %xmm0, %xmm7
	{store} vmovdqa64 %xmm0, %xmm7
	vmovdqu %xmm0, %xmm7
	{load} vmovdqu %xmm0, %xmm7
	{store} vmovdqu %xmm0, %xmm7
	vmovdqu8 %zmm0, %zmm7
	{load} vmovdqu8 %zmm0, %zmm7
	{store} vmovdqu8 %zmm0, %zmm7
	vmovdqu8 %xmm0, %xmm7
	{load} vmovdqu8 %xmm0, %xmm7
	{store} vmovdqu8 %zmm0, %zmm7
	vmovdqu16 %zmm0, %zmm7
	{load} vmovdqu16 %zmm0, %zmm7
	{store} vmovdqu16 %zmm0, %zmm7
	vmovdqu16 %xmm0, %xmm7
	{load} vmovdqu16 %xmm0, %xmm7
	{store} vmovdqu16 %xmm0, %xmm7
	vmovdqu32 %zmm0, %zmm7
	{load} vmovdqu32 %zmm0, %zmm7
	{store} vmovdqu32 %zmm0, %zmm7
	vmovdqu32 %xmm0, %xmm7
	{load} vmovdqu32 %xmm0, %xmm7
	{store} vmovdqu32 %xmm0, %xmm7
	vmovdqu64 %zmm0, %zmm7
	{load} vmovdqu64 %zmm0, %zmm7
	{store} vmovdqu64 %zmm0, %zmm7
	vmovdqu64 %xmm0, %xmm7
	{load} vmovdqu64 %xmm0, %xmm7
	{store} vmovdqu64 %xmm0, %xmm7
	vmovq %xmm0, %xmm7
	{load} vmovq %xmm0, %xmm7
	{store} vmovq %xmm0, %xmm7
	{evex} vmovq %xmm0, %xmm7
	{load} {evex} vmovq %xmm0, %xmm7
	{store} {evex} vmovq %xmm0, %xmm7

	pextrw $0, %xmm0, %edi
	{load} pextrw $0, %xmm0, %edi
	{store} pextrw $0, %xmm0, %edi

	vpextrw $0, %xmm0, %edi
	{load} vpextrw $0, %xmm0, %edi
	{store} vpextrw $0, %xmm0, %edi

	{evex} vpextrw $0, %xmm0, %edi
	{load} {evex} vpextrw $0, %xmm0, %edi
	{store} {evex} vpextrw $0, %xmm0, %edi

	bndmov %bnd3, %bnd0
	{load} bndmov %bnd3, %bnd0
	{store} bndmov %bnd3, %bnd0

	movaps (%rax),%xmm2
	{load} movaps (%rax),%xmm2
	{store} movaps (%rax),%xmm2
	{disp8} movaps (%rax),%xmm2
	{disp32} movaps (%rax),%xmm2
	movaps -1(%rax),%xmm2
	{disp8} movaps -1(%rax),%xmm2
	{disp32} movaps -1(%rax),%xmm2
	movaps 128(%rax),%xmm2
	{disp8} movaps 128(%rax),%xmm2
	{disp32} movaps 128(%rax),%xmm2
	{rex} mov %al,%ah
	{rex} shl %cl, %eax
	{rex} movabs 1, %al
	{rex} cmp %cl, %dl
	{rex} mov $1, %bl
	{rex} crc32 %cl, %eax
	{rex} movl %eax,%ebx
	{rex} movl %eax,%r14d
	{rex} movl %eax,(%r8)
	{rex} movaps %xmm7,%xmm2
	{rex} movaps %xmm7,%xmm12
	{rex} movaps (%rcx),%xmm2
	{rex} movaps (%r8),%xmm2
	{rex} phaddw (%rcx),%mm0
	{rex} phaddw (%r8),%mm0

	movb (%rbp),%al
	{disp8} movb (%rbp),%al
	{disp32} movb (%rbp),%al

	movb (%ebp),%al
	{disp8} movb (%ebp),%al
	{disp32} movb (%ebp),%al

	movb (%r13),%al
	{disp8} movb (%r13),%al
	{disp32} movb (%r13),%al

	movb (%r13d),%al
	{disp8} movb (%r13d),%al
	{disp32} movb (%r13d),%al

	.intel_syntax noprefix
	{vex3} vmovaps xmm2,xmm7
	{vex3} {load} vmovaps xmm2,xmm7
	{vex3} {store} vmovaps xmm2,xmm7
	vmovaps xmm2,xmm7
	{vex2} vmovaps xmm2,xmm7
	{vex2} {load} vmovaps xmm2,xmm7
	{vex2} {store} vmovaps xmm2,xmm7
	{vex3} vmovaps xmm2,XMMWORD PTR [rax]
	vmovaps xmm2,XMMWORD PTR [rax]
	{vex2} vmovaps xmm2,XMMWORD PTR [rax]
	{evex} vmovaps xmm2,XMMWORD PTR [rax]
	{disp32} vmovaps xmm2,XMMWORD PTR [rax]
	{evex} {disp8} vmovaps xmm2,XMMWORD PTR [rax]
	{evex} {disp32} vmovaps xmm2,XMMWORD PTR [rax]

	{vex} {disp8} vmovaps xmm2,XMMWORD PTR [rax+128]
	{vex} {disp32} vmovaps xmm2,XMMWORD PTR [rax+128]
	{evex} {disp8} vmovaps xmm2,XMMWORD PTR [rax+128]
	{evex} {disp32} vmovaps xmm2,XMMWORD PTR [rax+128]

	mov rax,rcx
	{load} mov rax,rcx
	{store} mov rax,rcx
	movaps xmm2,XMMWORD PTR [rax]
	{load} movaps xmm2,XMMWORD PTR [rax]
	{store} movaps xmm2,XMMWORD PTR [rax]
	{disp8} movaps xmm2,XMMWORD PTR [rax]
	{disp32} movaps xmm2,XMMWORD PTR [rax]
	movaps xmm2,XMMWORD PTR [rax-1]
	{disp8} movaps xmm2,XMMWORD PTR [rax-1]
	{disp32} movaps xmm2,XMMWORD PTR [rax-1]
	movaps xmm2,XMMWORD PTR [rax+128]
	{disp8} movaps xmm2,XMMWORD PTR [rax+128]
	{disp32} movaps xmm2,XMMWORD PTR [rax+128]
	{rex} mov ah,al
	{rex} mov ebx,eax
	{rex} mov r14d,eax
	{rex} mov DWORD PTR [r8],eax
	{rex} movaps xmm2,xmm7
	{rex} movaps xmm12,xmm7
	{rex} movaps xmm2,XMMWORD PTR [rcx]
	{rex} movaps xmm2,XMMWORD PTR [r8]
	{rex} phaddw mm0,QWORD PTR [rcx]
	{rex} phaddw mm0,QWORD PTR [r8]

	mov al, BYTE PTR [rbp]
	{disp8} mov al, BYTE PTR [rbp]
	{disp32} mov al, BYTE PTR [rbp]

	mov al, BYTE PTR [ebp]
	{disp8} mov al, BYTE PTR [ebp]
	{disp32} mov al, BYTE PTR [ebp]

	mov al, BYTE PTR [r13]
	{disp8} mov al, BYTE PTR [r13]
	{disp32} mov al, BYTE PTR [r13]

	mov al, BYTE PTR [r13]
	{disp8} mov al, BYTE PTR [r13d]
	{disp32} mov al, BYTE PTR [r13d]
