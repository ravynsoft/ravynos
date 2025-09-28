        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .text
        .align  1
        .thumb
        .thumb_func
        .global _start
_start:
        @ Create a situation where a multiple-load that should be
        @ patched cannot be, due to its belonging to an IT block
        @ but not in last position, which is the only position
        @ when a branch is valid in a IT block
        itt eq
        ldmeq.w  r9, {r1-r9}
        nop.w
