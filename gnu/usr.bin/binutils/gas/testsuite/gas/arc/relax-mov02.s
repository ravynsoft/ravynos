;;; Check relaxation from mov rA,limm -> mov_s rA, u8
	.cpu EM
	nop_s
	mov	r1, @.L1 - .
	nop
	nop
.L1:
	add	r0,r0,r0
