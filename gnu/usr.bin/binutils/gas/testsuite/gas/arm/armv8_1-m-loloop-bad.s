	.syntax unified
	.text
	.thumb
foo:
	wls r1, r2, .LB1
	dls r2, r2
	dls lr, pc
	le lr, #4096
	le #-4098
	le #-4095
.LB1:
	mov r3, r2
