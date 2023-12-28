;;; Check relaxation from mpy rA,rB,limm -> mpy rA,rB, uimm6@pcl
	.cpu EM
	mpy 	r1,r5,@.L1 - .
	nop
	nop
	nop
.L1:
	add	r0,r0,r0
