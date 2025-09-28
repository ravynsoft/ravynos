	.text
	.global	gc76fn
	.type	gc76fn,@function
gc76fn:
	move.d	gc76var:GOT,$r10
.Lfe:
	.size	gc76fn,.Lfe-gc76fn

	.data
	.type	gc76var,@object
	.size	gc76var,4
gc76var:
	.dword 0

	.section .text.2,"ax"
	.global	tlsdsofn
	.type	tlsdsofn,@function
	.p2align 1
tlsdsofn:
	move.d x:DTPREL,$r10
	move.d x:DTPREL,$r10
.Lfe2:
	.size	tlsdsofn,.Lfe2-tlsdsofn
