	.syntax unified
	.text
	.thumb
foo:
	bfl 2, 6
	mov r0, r1
	bfl .LBranch, .LB2
	mov r2, r1
.LB2:
	mov r3, r2
.LBranch:
	mov r4, r2
