	.text
	.global	tlsdsofn
	.type	tlsdsofn,@function
	.p2align 1
tlsdsofn:
	move.d x1:DTPREL,$r10
	move.d x2:DTPREL,$r11
.Lfe7:
	.size	tlsdsofn,.Lfe7-tlsdsofn
