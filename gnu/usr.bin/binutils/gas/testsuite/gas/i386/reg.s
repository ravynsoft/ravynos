# Check instructions with one register operand

	.text
_start:
psrlw $2, %mm6
psrlw $2, %xmm6
psraw $2, %mm6
psraw $2, %xmm6
psllw $2, %mm6
psllw $2, %xmm6
psrld $2, %mm6
psrld $2, %xmm6
psrad $2, %mm6
psrad $2, %xmm6
pslld $2, %mm6
pslld $2, %xmm6
psrlq $2, %mm6
psrlq $2, %xmm6
psrldq $2, %xmm6
psllq $2, %mm6
psllq $2, %xmm6
pslldq $2, %xmm6

.intel_syntax noprefix
psrlw mm6, 2
psrlw xmm6, 2
psraw mm6, 2
psraw xmm6, 2
psllw mm6, 2
psllw xmm6, 2
psrld mm6, 2
psrld xmm6, 2
psrad mm6, 2
psrad xmm6, 2
pslld mm6, 2
pslld xmm6, 2
psrlq mm6, 2
psrlq xmm6, 2
psrldq xmm6, 2
psllq mm6, 2
psllq xmm6, 2
pslldq xmm6, 2

.p2align 4,0
