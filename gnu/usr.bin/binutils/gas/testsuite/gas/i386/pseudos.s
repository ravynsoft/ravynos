# Check instructions with pseudo prefixes for encoding

	.text
_start:
	{vex3} vmovaps %xmm7,%xmm2
	{vex3} {load} vmovaps %xmm7,%xmm2
	{vex3} {store} vmovaps %xmm7,%xmm2
	vmovaps %xmm7,%xmm2
	{vex} vmovaps %xmm7,%xmm2
	{vex} {load} vmovaps %xmm7,%xmm2
	{vex} {store} vmovaps %xmm7,%xmm2
	{vex3} vmovaps (%eax),%xmm2
	vmovaps (%eax),%xmm2
	{vex2} vmovaps (%eax),%xmm2
	{evex} vmovaps (%eax),%xmm2
	{disp32} vmovaps (%eax),%xmm2
	{evex} {disp8} vmovaps (%eax),%xmm2
	{evex} {disp32} vmovaps (%eax),%xmm2

	{vex} {disp8} vmovaps 128(%eax),%xmm2
	{vex} {disp32} vmovaps 128(%eax),%xmm2
	{evex} {disp8} vmovaps 128(%eax),%xmm2
	{evex} {disp16} vmovaps 128(%bx),%xmm2
	{evex} {disp32} vmovaps 128(%eax),%xmm2

	mov %ecx, %eax
	{load} mov %ecx, %eax
	{store} mov %ecx, %eax
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
	{load} mov %eax, (%edi)
	{load} mov (%edi), %eax
	{store} mov %eax, (%edi)
	{store} mov (%edi), %eax
	{load} mov %es, %edi
	{load} mov %eax, %gs
	{store} mov %es, %edi
	{store} mov %eax, %gs
	{load} mov %cr0, %edi
	{load} mov %eax, %cr7
	{store} mov %cr0, %edi
	{store} mov %eax, %cr7
	{load} mov %dr0, %edi
	{load} mov %eax, %dr7
	{store} mov %dr0, %edi
	{store} mov %eax, %dr7
	{load} kmovb %k0, %edi
	{load} kmovb %eax, %k7
	{store} kmovb %k0, %edi
	{store} kmovb %eax, %k7
	{load} kmovd %k0, %edi
	{load} kmovd %eax, %k7
	{store} kmovd %k0, %edi
	{store} kmovd %eax, %k7
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
	{load} adc %eax, (%edi)
	{load} adc (%edi), %eax
	{store} adc %eax, (%edi)
	{store} adc (%edi), %eax
	{load} add %eax, (%edi)
	{load} add (%edi), %eax
	{store} add %eax, (%edi)
	{store} add (%edi), %eax
	{load} and %eax, (%edi)
	{load} and (%edi), %eax
	{store} and %eax, (%edi)
	{store} and (%edi), %eax
	{load} cmp %eax, (%edi)
	{load} cmp (%edi), %eax
	{store} cmp %eax, (%edi)
	{store} cmp (%edi), %eax
	{load} or %eax, (%edi)
	{load} or (%edi), %eax
	{store} or %eax, (%edi)
	{store} or (%edi), %eax
	{load} sbb %eax, (%edi)
	{load} sbb (%edi), %eax
	{store} sbb %eax, (%edi)
	{store} sbb (%edi), %eax
	{load} sub %eax, (%edi)
	{load} sub (%edi), %eax
	{store} sub %eax, (%edi)
	{store} sub (%edi), %eax
	{load} xor %eax, (%edi)
	{load} xor (%edi), %eax
	{store} xor %eax, (%edi)
	{store} xor (%edi), %eax

	.irp m, mov, adc, add, and, cmp, or, sbb, sub, test, xor
	\m	$0x12, %al
	\m	$0x345, %eax
	{load} \m $0x12, %al		# bogus for MOV
	{load} \m $0x345, %eax		# bogus for MOV
	{store} \m $0x12, %al
	{store} \m $0x345, %eax
	.endr

	.irp m, inc, dec, push, pop, bswap
	\m	%ecx
	{load} \m %ecx			# bogus for POP
	{store} \m %ecx			# bogus for PUSH
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

	movaps (%eax),%xmm2
	{load} movaps (%eax),%xmm2
	{store} movaps (%eax),%xmm2
	{disp8} movaps (%eax),%xmm2
	{disp32} movaps (%eax),%xmm2
	movaps -1(%eax),%xmm2
	{disp8} movaps -1(%eax),%xmm2
	{disp32} movaps -1(%eax),%xmm2
	movaps 128(%eax),%xmm2
	{disp8} movaps 128(%eax),%xmm2
	{disp32} movaps 128(%eax),%xmm2

	movb (%ebp),%al
	{disp8} movb (%ebp),%al
	{disp32} movb (%ebp),%al

	movb (%si),%al
	{disp8} movb (%si),%al
	{disp16} movb (%si),%al

	movb (%di),%al
	{disp8} movb (%di),%al
	{disp16} movb (%di),%al

	movb (%bx),%al
	{disp8} movb (%bx),%al
	{disp16} movb (%bx),%al

	movb (%bp),%al
	{disp8} movb (%bp),%al
	{disp16} movb (%bp),%al

	.intel_syntax noprefix
	{vex3} vmovaps xmm2,xmm7
	{vex3} {load} vmovaps xmm2,xmm7
	{vex3} {store} vmovaps xmm2,xmm7
	vmovaps xmm2,xmm7
	{vex2} vmovaps xmm2,xmm7
	{vex2} {load} vmovaps xmm2,xmm7
	{vex2} {store} vmovaps xmm2,xmm7
	{vex3} vmovaps xmm2,XMMWORD PTR [eax]
	vmovaps xmm2,XMMWORD PTR [eax]
	{vex2} vmovaps xmm2,XMMWORD PTR [eax]
	{evex} vmovaps xmm2,XMMWORD PTR [eax]
	{disp32} vmovaps xmm2,XMMWORD PTR [eax]
	{evex} {disp8} vmovaps xmm2,XMMWORD PTR [eax]
	{evex} {disp32} vmovaps xmm2,XMMWORD PTR [eax]

	{vex} {disp8} vmovaps xmm2,XMMWORD PTR [eax+128]
	{vex} {disp32} vmovaps xmm2,XMMWORD PTR [eax+128]
	{evex} {disp8} vmovaps xmm2,XMMWORD PTR [eax+128]
	{evex} {disp16} vmovaps xmm2,XMMWORD PTR [bx+128]
	{evex} {disp32} vmovaps xmm2,XMMWORD PTR [eax+128]

	mov eax,ecx
	{load} mov eax,ecx
	{store} mov eax,ecx
	movaps xmm2,XMMWORD PTR [eax]
	{load} movaps xmm2,XMMWORD PTR [eax]
	{store} movaps xmm2,XMMWORD PTR [eax]
	{disp8} movaps xmm2,XMMWORD PTR [eax]
	{disp32} movaps xmm2,XMMWORD PTR [eax]
	movaps xmm2,XMMWORD PTR [eax-1]
	{disp8} movaps xmm2,XMMWORD PTR [eax-1]
	{disp32} movaps xmm2,XMMWORD PTR [eax-1]
	movaps xmm2,XMMWORD PTR [eax+128]
	{disp8} movaps xmm2,XMMWORD PTR [eax+128]
	{disp32} movaps xmm2,XMMWORD PTR [eax+128]

	mov al, BYTE PTR [ebp]
	{disp8} mov al, BYTE PTR [ebp]
	{disp32} mov al, BYTE PTR [ebp]

	mov al, BYTE PTR [si]
	{disp8} mov al, BYTE PTR [si]
	{disp16} mov al, BYTE PTR [si]

	mov al, BYTE PTR [di]
	{disp8} mov al, BYTE PTR [di]
	{disp16} mov al, BYTE PTR [di]

	mov al, BYTE PTR [bx]
	{disp8} mov al, BYTE PTR [bx]
	{disp16} mov al, BYTE PTR [bx]

	mov al, BYTE PTR [bp]
	{disp8} mov al, BYTE PTR [bp]
	{disp16} mov al, BYTE PTR [bp]

	{disp32} jmp .
	.code16
	{disp16} jmp .
	.byte -1, -1
