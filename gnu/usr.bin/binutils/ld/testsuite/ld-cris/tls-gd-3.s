	.text
	.global	tlsfn
	.type	tlsfn,@function
	.p2align 1
tlsfn:
	move.d x:GD,$r10
.Lfe1:
	.size	tlsfn,.Lfe1-tlsfn
