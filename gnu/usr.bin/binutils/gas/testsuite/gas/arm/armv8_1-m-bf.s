	.syntax unified
	.text
	.thumb
foo:
	bf 2, 6
	mov r1, r1
	bf .LBranch, .LB2
	mov r2, r1
.LB2:
	mov r3, r2
.LBranch:
	mov r4, r2
