;;; Check relaxation from sub rA,rB,limm@pcl -> sub_s rC,rB, uimm3@pcl
	.cpu EM
	nop
	sub 	r1,r2,@.L1@pcl
	nop
	.align 4
.L1:
	add 	r0,r0,r0
