	.syntax unified
	.cpu cortex-a8
	.text

	@ expansion 32 bytes
        .macro bw1
        add.w r0, r1, r2
        blx.w armfn
        add.w r0, r1, r2
        blx.w armfn
        add.w r0, r1, r2
        blx.w armfn
        add.w r0, r1, r2
        blx.w armfn
        .endm

        @ expansion 128 bytes
        .macro bw2
        bw1
        bw1
        bw1
        bw1
        .endm

	.arm
        .align  2
armfn:
	mov	r2, r3, asl r4
	bx	lr

        .global _start

	.thumb
        .thumb_func
	.align 3
        .type   _start, %function
_start:
        nop

	@ Trigger Cortex-A8 erratum workaround with blx instructions.
        bw2
        bw2

        bx      lr
