;;; Check relaxation from ld rA,[rB,limm@pcl] -> ld_s rC,[rB, imm9@pcl]
	.cpu EM
	nop_s
	ld 	r1,[r2,@.L1@pcl]
	nop
	nop
	.align 4
.L1:
	add 	r0,r0,r0
