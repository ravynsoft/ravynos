	.text

	.ifndef r
	.set r,1
	.endif

	.global	tlsdsofndtprelm
	.type	tlsdsofndtprelm,@function
	.p2align 1
tlsdsofndtprelm:
	move.w x:DTPREL16,$r10
.Lfe:
	.size	tlsdsofndtprelm,.Lfe-tlsdsofndtprelm

	.globl z
	.section	.tdata,"awT",@progbits
	.p2align 2
	.type	z,@object
	.size	z,r
z:
	.fill	r,1,42

	.type	x,@object
	.size	x,1
x:
	.byte 42
