	.include "tls128.s"
	.text
	.global	tlsdsofn2
	.type	tlsdsofn2,@function
	.p2align 1
tlsdsofn2:
	move.d tls128+42:TPOFFGOT,$r10
.Lfe1:
	.size	tlsdsofn2,.Lfe1-tlsdsofn2
