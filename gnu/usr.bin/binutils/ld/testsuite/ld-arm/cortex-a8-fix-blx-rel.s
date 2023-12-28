	.syntax unified
	.cpu cortex-a8
	.thumb
	.text

	@ expansion 32 bytes
        .macro bw1
1:
        add.w r0, r1, r2
        blx.w targetfn
        add.w r0, r1, r2
        blx.w targetfn
        add.w r0, r1, r2
        blx.w targetfn
        add.w r0, r1, r2
        blx.w targetfn
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

	bw2
	bw2

        bx      lr
