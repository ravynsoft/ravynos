	.text
	.global	tlsdsofn
	.type	tlsdsofn,@function
	.p2align 1
tlsdsofn:
	move.w x:TPOFFGOT16,$r10
.Lfe8:
	.size	tlsdsofn,.Lfe8-tlsdsofn
