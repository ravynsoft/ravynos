	.data
_start:
.set	x, .-_start
.long	x
.balign 4
	.set	x, .-_start
	.long	x
.L_xx:	.set	x, .-_start
	.long	x
.L_yy:.set	x, .-_start
	.long	x
