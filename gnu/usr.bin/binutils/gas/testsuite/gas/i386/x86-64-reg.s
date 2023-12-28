# Check 64bit instructions with one register operand

	.text
_start:
psrlw $2, %mm6
psrlw $2, %xmm10
psraw $2, %mm6
psraw $2, %xmm10
psllw $2, %mm6
psllw $2, %xmm10
psrld $2, %mm6
psrld $2, %xmm10
psrad $2, %mm6
psrad $2, %xmm10
pslld $2, %mm6
pslld $2, %xmm10
psrlq $2, %mm6
psrlq $2, %xmm10
psrldq $2, %xmm10
psllq $2, %mm6
psllq $2, %xmm10
pslldq $2, %xmm10

	add	$1, %axl
	add	$1, %cxl
	add	$1, %dxl
	add	$1, %bxl
	add	$1, %spl
	add	$1, %bpl
	add	$1, %sil
	add	$1, %dil

.intel_syntax noprefix
psrlw mm6, 2
psrlw xmm2, 2
psraw mm6, 2
psraw xmm2, 2
psllw mm6, 2
psllw xmm2, 2
psrld mm6, 2
psrld xmm2, 2
psrad mm6, 2
psrad xmm2, 2
pslld mm6, 2
pslld xmm2, 2
psrlq mm6, 2
psrlq xmm2, 2
psrldq xmm2, 2
psllq mm6, 2
psllq xmm2, 2
pslldq xmm2, 2

.p2align 4,0
