# Test ck801 jbf/jbt branch relaxation.

	addu a1, a2, a3
.L1:
	jbf .L2
	addu a3, a2, a1

	.rept 600
	nop
	.endr


	addu a1, a2, a3
.L2:
	jbt .L1
	addu a3, a2, a1
