	.syntax unified
	.text
	.thumb
foo:
.Lstart:
	wls lr, r2, .LB1
	dls lr, r2
	dls lr, lr
	le lr, .Lstart
	le .Lstart
	le lr, #-1172
	le #-12
.LB1:
	mov r3, r2
