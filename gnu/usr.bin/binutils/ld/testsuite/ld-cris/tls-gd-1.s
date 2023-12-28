	.text
	.global	tlsdsofn0
	.type	tlsdsofn0,@function
	.p2align 1
tlsdsofn0:
	move.w x:GDGOTREL16,$r10
.Lfe:
	.size	tlsdsofn0,.Lfe-tlsdsofn0
