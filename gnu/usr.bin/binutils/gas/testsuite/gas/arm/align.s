	.syntax unified
	.thumb
	.global foo
foo:	
	nop
	mov r1,r2
	.p2align 4
	mov r1,r2
	.p2align 3

	.arm
	.global bar
bar:	
	nop
	mov r1,r2
	.p2align 4
	mov r1,r2
	.p2align 5
