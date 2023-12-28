	.text
	.global	tlsdsofn9
	.type	tlsdsofn9,@function
	.p2align 1
tlsdsofn9:
	move.w x1:TPOFFGOT16,$r10
	move.w x2:TPOFFGOT16,$r11
.Lfe9:
	.size	tlsdsofn9,.Lfe9-tlsdsofn9
