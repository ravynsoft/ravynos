        .syntax unified
        .text
        cmp     r0, #0
        itt     eq
        moveq   r0, r1
.section second
	itt ne
	movne r0, r1

