	.text
	.global	tlsdsofn
	.type	tlsdsofn,@function
	.p2align 1
tlsdsofn:
	move.w x1:DTPREL16,$r10
	move.w x2:DTPREL16,$r11
.Lfe5:
	.size	tlsdsofn,.Lfe5-tlsdsofn

