;;; Check relaxation from add rA,rB,limm@pcl -> add_s rC,B, uimm3@pcl
	.cpu EM
	nop
	add 	r1,r2,@.L1@pcl
	nop
	.align 4
.L1:
	add	r0,r0,r0
