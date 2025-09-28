;;; Check relaxation from add rA,rB,limm (PCL) -> add rA,rB, uimm6@pcl
	.cpu EM
	add 	r1,r5,@.L1 - .
	nop
	nop
	nop
.L1:
	add 	r0,r0,r0
