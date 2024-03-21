# Check instructions with encoding options

	.allow_index_reg
	.text
_start:

# Tests for op reg, reg
	add %dl,%cl
	add.s %dl,%cl
	add %dx,%cx
	add.s %dx,%cx
	add %edx,%ecx
	add.s %edx,%ecx
	addb %dl,%cl
	addb.s %dl,%cl
	addw %dx,%cx
	addw.s %dx,%cx
	addl %edx,%ecx
	addl.s %edx,%ecx
	adc %dl,%cl
	adc.s %dl,%cl
	adc %dx,%cx
	adc.s %dx,%cx
	adc %edx,%ecx
	adc.s %edx,%ecx
	adcb %dl,%cl
	adcb.s %dl,%cl
	adcw %dx,%cx
	adcw.s %dx,%cx
	adcl %edx,%ecx
	adcl.s %edx,%ecx
	and %dl,%cl
	and.s %dl,%cl
	and %dx,%cx
	and.s %dx,%cx
	and %edx,%ecx
	and.s %edx,%ecx
	andb %dl,%cl
	andb.s %dl,%cl
	andw %dx,%cx
	andw.s %dx,%cx
	andl %edx,%ecx
	andl.s %edx,%ecx
	cmp %dl,%cl
	cmp.s %dl,%cl
	cmp %dx,%cx
	cmp.s %dx,%cx
	cmp %edx,%ecx
	cmp.s %edx,%ecx
	cmpb %dl,%cl
	cmpb.s %dl,%cl
	cmpw %dx,%cx
	cmpw.s %dx,%cx
	cmpl %edx,%ecx
	cmpl.s %edx,%ecx
	mov %dl,%cl
	mov.s %dl,%cl
	mov %dx,%cx
	mov.s %dx,%cx
	mov %edx,%ecx
	mov.s %edx,%ecx
	movb %dl,%cl
	movb.s %dl,%cl
	movw %dx,%cx
	movw.s %dx,%cx
	movl %edx,%ecx
	movl.s %edx,%ecx
	or %dl,%cl
	or.s %dl,%cl
	or %dx,%cx
	or.s %dx,%cx
	or %edx,%ecx
	or.s %edx,%ecx
	orb %dl,%cl
	orb.s %dl,%cl
	orw %dx,%cx
	orw.s %dx,%cx
	orl %edx,%ecx
	orl.s %edx,%ecx
	sbb %dl,%cl
	sbb.s %dl,%cl
	sbb %dx,%cx
	sbb.s %dx,%cx
	sbb %edx,%ecx
	sbb.s %edx,%ecx
	sbbb %dl,%cl
	sbbb.s %dl,%cl
	sbbw %dx,%cx
	sbbw.s %dx,%cx
	sbbl %edx,%ecx
	sbbl.s %edx,%ecx
	sub %dl,%cl
	sub.s %dl,%cl
	sub %dx,%cx
	sub.s %dx,%cx
	sub %edx,%ecx
	sub.s %edx,%ecx
	subb %dl,%cl
	subb.s %dl,%cl
	subw %dx,%cx
	subw.s %dx,%cx
	subl %edx,%ecx
	subl.s %edx,%ecx
	xor %dl,%cl
	xor.s %dl,%cl
	xor %dx,%cx
	xor.s %dx,%cx
	xor %edx,%ecx
	xor.s %edx,%ecx
	xorb %dl,%cl
	xorb.s %dl,%cl
	xorw %dx,%cx
	xorw.s %dx,%cx
	xorl %edx,%ecx
	xorl.s %edx,%ecx

# Tests for op ymm, ymm
	vmovapd %ymm4,%ymm6
	vmovapd.s %ymm4,%ymm6
	vmovaps %ymm4,%ymm6
	vmovaps.s %ymm4,%ymm6
	vmovdqa %ymm4,%ymm6
	vmovdqa.s %ymm4,%ymm6
	vmovdqu %ymm4,%ymm6
	vmovdqu.s %ymm4,%ymm6
	vmovupd %ymm4,%ymm6
	vmovupd.s %ymm4,%ymm6
	vmovups %ymm4,%ymm6
	vmovups.s %ymm4,%ymm6

# Tests for op xmm, xmm
	movapd %xmm4,%xmm6
	movapd.s %xmm4,%xmm6
	movaps %xmm4,%xmm6
	movaps.s %xmm4,%xmm6
	movdqa %xmm4,%xmm6
	movdqa.s %xmm4,%xmm6
	movdqu %xmm4,%xmm6
	movdqu.s %xmm4,%xmm6
	movq %xmm4,%xmm6
	movq.s %xmm4,%xmm6
	movsd %xmm4,%xmm6
	movsd.s %xmm4,%xmm6
	movss %xmm4,%xmm6
	movss.s %xmm4,%xmm6
	movupd %xmm4,%xmm6
	movupd.s %xmm4,%xmm6
	movups %xmm4,%xmm6
	movups.s %xmm4,%xmm6
	vmovapd %xmm4,%xmm6
	vmovapd.s %xmm4,%xmm6
	vmovaps %xmm4,%xmm6
	vmovaps.s %xmm4,%xmm6
	vmovdqa %xmm4,%xmm6
	vmovdqa.s %xmm4,%xmm6
	vmovdqu %xmm4,%xmm6
	vmovdqu.s %xmm4,%xmm6
	vmovq %xmm4,%xmm6
	vmovq.s %xmm4,%xmm6
	vmovupd %xmm4,%xmm6
	vmovupd.s %xmm4,%xmm6
	vmovups %xmm4,%xmm6
	vmovups.s %xmm4,%xmm6

# Tests for op xmm, xmm, xmm
	vmovsd %xmm4,%xmm6,%xmm2
	vmovsd.s %xmm4,%xmm6,%xmm2
	vmovss %xmm4,%xmm6,%xmm2
	vmovss.s %xmm4,%xmm6,%xmm2

# Tests for op mm, mm
	movq %mm0,%mm4
	movq.s %mm0,%mm4

# Tests for op bnd, bnd
	bndmov %bnd1,%bnd2
	bndmov.s %bnd1,%bnd2

	.intel_syntax noprefix

# Tests for op reg, reg
	add cl,dl
	add.s cl,dl
	add cx,dx
	add.s cx,dx
	add ecx,edx
	add.s ecx,edx
	adc cl,dl
	adc.s cl,dl
	adc cx,dx
	adc.s cx,dx
	adc ecx,edx
	adc.s ecx,edx
	and cl,dl
	and.s cl,dl
	and cx,dx
	and.s cx,dx
	and ecx,edx
	and.s ecx,edx
	cmp cl,dl
	cmp.s cl,dl
	cmp cx,dx
	cmp.s cx,dx
	cmp ecx,edx
	cmp.s ecx,edx
	mov cl,dl
	mov.s cl,dl
	mov cx,dx
	mov.s cx,dx
	mov ecx,edx
	mov.s ecx,edx
	or cl,dl
	or.s cl,dl
	or cx,dx
	or.s cx,dx
	or ecx,edx
	or.s ecx,edx
	sbb cl,dl
	sbb.s cl,dl
	sbb cx,dx
	sbb.s cx,dx
	sbb ecx,edx
	sbb.s ecx,edx
	sub cl,dl
	sub.s cl,dl
	sub cx,dx
	sub.s cx,dx
	sub ecx,edx
	sub.s ecx,edx
	xor cl,dl
	xor.s cl,dl
	xor cx,dx
	xor.s cx,dx
	xor ecx,edx
	xor.s ecx,edx

# Tests for op ymm, ymm
	vmovapd ymm6,ymm4
	vmovapd.s ymm6,ymm4
	vmovaps ymm6,ymm4
	vmovaps.s ymm6,ymm4
	vmovdqa ymm6,ymm4
	vmovdqa.s ymm6,ymm4
	vmovdqu ymm6,ymm4
	vmovdqu.s ymm6,ymm4
	vmovupd ymm6,ymm4
	vmovupd.s ymm6,ymm4
	vmovups ymm6,ymm4
	vmovups.s ymm6,ymm4

# Tests for op xmm, xmm
	movapd xmm6,xmm4
	movapd.s xmm6,xmm4
	movaps xmm6,xmm4
	movaps.s xmm6,xmm4
	movdqa xmm6,xmm4
	movdqa.s xmm6,xmm4
	movdqu xmm6,xmm4
	movdqu.s xmm6,xmm4
	movq xmm6,xmm4
	movq.s xmm6,xmm4
	movsd xmm6,xmm4
	movsd.s xmm6,xmm4
	movss xmm6,xmm4
	movss.s xmm6,xmm4
	movupd xmm6,xmm4
	movupd.s xmm6,xmm4
	movups xmm6,xmm4
	movups.s xmm6,xmm4
	vmovapd xmm6,xmm4
	vmovapd.s xmm6,xmm4
	vmovaps xmm6,xmm4
	vmovaps.s xmm6,xmm4
	vmovdqa xmm6,xmm4
	vmovdqa.s xmm6,xmm4
	vmovdqu xmm6,xmm4
	vmovdqu.s xmm6,xmm4
	vmovq xmm6,xmm4
	vmovq.s xmm6,xmm4
	vmovupd xmm6,xmm4
	vmovupd.s xmm6,xmm4
	vmovups xmm6,xmm4
	vmovups.s xmm6,xmm4

# Tests for op xmm, xmm, xmm
	vmovsd xmm2,xmm6,xmm4
	vmovsd.s xmm2,xmm6,xmm4
	vmovss xmm2,xmm6,xmm4
	vmovss.s xmm2,xmm6,xmm4

# Tests for op mm, mm
	movq mm4,mm0
	movq.s mm4,mm0

# Tests for op bnd, bnd
	bndmov bnd1,bnd2
	bndmov.s bnd1,bnd2
