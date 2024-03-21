	.text
	.global	tlsdsofn
	.type	tlsdsofn,@function
	.p2align 1
tlsdsofn:
	move.w x:DTPREL16,$r10
.Lfe1:
	.size	tlsdsofn,.Lfe1-tlsdsofn
