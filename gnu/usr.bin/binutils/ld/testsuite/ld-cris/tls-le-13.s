	.text
	.global	tlsfn13
	.type	tlsfn13,@function
	.p2align 1
tlsfn13:
	move.d x1:TPOFF,$r10
	move.d x2:TPOFF,$r10
.Lfe13:
	.size	tlsfn13,.Lfe13-tlsfn13
