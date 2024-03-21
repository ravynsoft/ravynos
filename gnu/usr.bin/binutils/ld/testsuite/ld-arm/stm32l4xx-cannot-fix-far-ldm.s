        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .text
        .align  1
        .thumb
        .thumb_func
        .global _start
_start:
        .space 0xFFFF00

        @ Multiple load, case #2
        @ ldm rx, {...} ->
        @ mov ry, rx where ry is the lowest register from upper_list
        @ ldm ry!, { lower_list }
        @ ldm ry,  { upper_list }
        ldm.w  r0, {r1-r9}

	.space 0x100

        @ Check that the linker never generates a wrong branch
        @ ldm rx, {...} -> ldm rx, {...}
        @ Emit a warning during the link phase and keep the same instruction

	ldm.w  r9, {r1-r9}

        nop
