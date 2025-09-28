;;; Check relaxation from sub rA,rB,limm (PCL) -> sub rA,rB, uimm6@pcl
	.cpu EM
	nop_s
	sub 	r1,pcl,@.L1 - .
	nop
	.align 4
.L1:
	add	r0,r0,r0
