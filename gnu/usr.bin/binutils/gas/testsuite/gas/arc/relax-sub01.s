;;; Check relaxation from sub rA,rB,limm@pcl -> sub rA,rB, uimm6@pcl
	.cpu EM
	sub 	r1,pcl,@.L1@pcl
	nop
	nop
	nop
	.align 4
.L1:
	add	r0,r0,r0
