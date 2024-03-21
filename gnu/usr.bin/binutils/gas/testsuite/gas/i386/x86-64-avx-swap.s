# Check 64bit instructions with encoding options

	.allow_index_reg
	.text
_start:

# Tests for op ymm, ymm
	vmovapd %ymm8,%ymm6
	vmovaps %ymm8,%ymm6
	vmovdqa %ymm8,%ymm6
	vmovdqu %ymm8,%ymm6
	vmovupd %ymm8,%ymm6
	vmovups %ymm8,%ymm6

# Tests for op xmm, xmm
	movapd %xmm8,%xmm6
	movaps %xmm8,%xmm6
	movdqa %xmm8,%xmm6
	movdqu %xmm8,%xmm6
	movq %xmm8,%xmm6
	movsd %xmm8,%xmm6
	movss %xmm8,%xmm6
	movupd %xmm8,%xmm6
	movups %xmm8,%xmm6
	vmovapd %xmm8,%xmm6
	vmovaps %xmm8,%xmm6
	vmovdqa %xmm8,%xmm6
	vmovdqu %xmm8,%xmm6
	vmovq %xmm8,%xmm6
	vmovupd %xmm8,%xmm6
	vmovups %xmm8,%xmm6

# Tests for op xmm, xmm, xmm
	vmovsd %xmm8,%xmm6,%xmm2
	vmovss %xmm8,%xmm6,%xmm2

	.intel_syntax noprefix

# Tests for op ymm, ymm
	vmovapd ymm6,ymm8
	vmovaps ymm6,ymm8
	vmovdqa ymm6,ymm8
	vmovdqu ymm6,ymm8
	vmovupd ymm6,ymm8
	vmovups ymm6,ymm8

# Tests for op xmm, xmm
	movapd xmm6,xmm8
	movaps xmm6,xmm8
	movdqa xmm6,xmm8
	movdqu xmm6,xmm8
	movq xmm6,xmm8
	movsd xmm6,xmm8
	movss xmm6,xmm8
	movupd xmm6,xmm8
	movups xmm6,xmm8
	vmovapd xmm6,xmm8
	vmovaps xmm6,xmm8
	vmovdqa xmm6,xmm8
	vmovdqu xmm6,xmm8
	vmovq xmm6,xmm8
	vmovupd xmm6,xmm8
	vmovups xmm6,xmm8

# Tests for op xmm, xmm, xmm
	vmovsd xmm2,xmm6,xmm8
	vmovss xmm2,xmm6,xmm8
