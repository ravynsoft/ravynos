	.text
	.global	tlsfn13
	.type	tlsfn13,@function
	.p2align 1
tlsfn13:
	move.w x1:TPOFF16,$r10
	move.w x2:TPOFF16,$r10
.Lfe13s:
	.size	tlsfn13,.Lfe13s-tlsfn13
