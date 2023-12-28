;;; Check relaxation from mov rA,limm -> mov rA, s12
	.cpu EM
	nop_s
	mov	r5, @.L1 - .
	nop
	nop
.L1:
	add	r0,r0,r0
