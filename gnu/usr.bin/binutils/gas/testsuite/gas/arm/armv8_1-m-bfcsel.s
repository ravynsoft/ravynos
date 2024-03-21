	.syntax unified
	.text
	.thumb
foo:
	bfcsel .LB1, .LBranch, .LB2, eq
	mov r1, r1
.LB1:
	beq .LBranch
.LB2:
	mov r3, r2
.LBranch:
	mov r4, r2
