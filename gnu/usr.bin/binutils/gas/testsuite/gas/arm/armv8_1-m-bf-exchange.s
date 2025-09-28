	.syntax unified
	.text
	.thumb
foo:
	bfx .LB1, r5
	mov r1, r1
	bflx .LB2, r3
	mov r2, r1
.LB1:
	mov r3, r2
.LB2:
	mov r4, r2
