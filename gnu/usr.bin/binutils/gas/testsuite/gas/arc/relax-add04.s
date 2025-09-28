;;; Check relaxation from add rA,rB,limm (PCL) -> add rA,rB, uimm6@pcl
	.cpu EM
	nop_s
	add 	r1,pcl,@.L1 - .
	nop
	.align 4
.L1:
	add	r0,r0,r0
