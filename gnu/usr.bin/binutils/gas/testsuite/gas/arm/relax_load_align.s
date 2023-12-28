@ The relaxation algorithm used to not compensate for alignment statements.
@ The early termination to avoid infinite looping would make the second load
@ a wide instruction.
	.text
	.thumb
	.syntax unified
fn:
	adds r0, r0, #1000
	ldr r0, 1f
	ldr r0, 1f
.align 2
1:
