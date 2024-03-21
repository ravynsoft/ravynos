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
        @ patched belongs to an IT block in the position where it can
        @ be, that is the last position in the IT block.
        @ Mostly to cover the IT detection logic.
        @ Tests correspond to LDM CASE #1.
        it eq
        ldmeq.w  r9, {r1-r9}

        itt eq
        nop.w
        ldmeq.w  r9, {r1-r9}

        ite eq
        nop.w
        ldmne.w  r9, {r1-r9}

        ittt eq
        nop.w
        nop.w
        ldmeq.w  r9, {r1-r9}

        itet eq
        nop.w
        nop.w
        ldmeq.w  r9, {r1-r9}

        itte eq
        nop.w
        nop.w
        ldmne.w  r9, {r1-r9}

        itee eq
        nop.w
        nop.w
        ldmne.w  r9, {r1-r9}

        itttt eq
        nop.w
        nop.w
        nop.w
        ldmeq.w  r9, {r1-r9}

        ittte eq
        nop.w
        nop.w
        nop.w
        ldmne.w  r9, {r1-r9}

        ittet eq
        nop.w
        nop.w
        nop.w
        ldmeq.w  r9, {r1-r9}

        ittee eq
        nop.w
        nop.w
        nop.w
        ldmne.w  r9, {r1-r9}

        itett eq
        nop.w
        nop.w
        nop.w
        ldmeq.w  r9, {r1-r9}

        itete eq
        nop.w
        nop.w
        nop.w
        ldmne.w  r9, {r1-r9}

        iteet eq
        nop.w
        nop.w
        nop.w
        ldmeq.w  r9, {r1-r9}

        iteee eq
        nop.w
        nop.w
        nop.w
        ldmne.w  r9, {r1-r9}
