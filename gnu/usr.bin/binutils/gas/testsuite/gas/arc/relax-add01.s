;;; Check relaxation from add rA,rB,limm@pcl -> add rA,rB, uimm6@pcl
	.cpu EM
	add 	r1,pcl,@.L1@pcl
	nop
	nop
	nop
	.align 4
.L1:
	add	r0,r0,r0
