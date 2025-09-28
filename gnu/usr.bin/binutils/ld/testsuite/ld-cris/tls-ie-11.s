	.text
	.global	tlsdsofn
	.type	tlsdsofn,@function
	.p2align 1
tlsdsofn:
	move.d x1:TPOFFGOT,$r10
	move.d x2:TPOFFGOT,$r11
.Lfe11:
	.size	tlsdsofn,.Lfe11-tlsdsofn
