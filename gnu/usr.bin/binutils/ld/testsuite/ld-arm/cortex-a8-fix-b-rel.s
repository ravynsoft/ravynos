	.syntax unified
	.cpu cortex-a8
	.thumb
	.text

	@ expansion 32 bytes
        .macro bw1
1:
        add.w r0, r1, r2
        b.w targetfn
        add.w r0, r1, r2
        b.w targetfn
        add.w r0, r1, r2
        b.w targetfn
        add.w r0, r1, r2
        b.w targetfn
        .endm

        @ expansion 128 bytes
        .macro bw2
        bw1
        bw1
        bw1
        bw1
        .endm

        .align  3
        .global _start
        .thumb
        .thumb_func
        .type   _start, %function
_start:
	nop

	@ If branching to an ARM destination, we *don't* want to create a
	@ Cortex-A8 stub: the Thumb-to-ARM stub will suffice (and we need it
	@ to change mode).
	bw2
	bw2

        bx      lr
