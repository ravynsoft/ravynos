        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .text
        .align  1
        .thumb
        .thumb_func
        .global _start
_start:
        @ VLDM CASE #1
        @ vldm rx, {...}
        @ -> vldm rx!, {8_words_or_less} for each
        @ -> sub rx, rx, #size (list)
        vldm r10, {d1-d15}

        @ VLDM CASE #2
        @ vldm rx!, {...}
        @ -> vldm rx!, {8_words_or_less} for each needed 8_word
        @ This also handles vpop instruction (when rx is sp)
        vldm r7!, {d5-d15}
        @ Explicit VPOP test
        vpop {d1-d5}

        @ VLDM CASE #3
        @ vldmd rx!, {...}
        @ -> vldmb rx!, {8_words_or_less} for each needed 8_word
        vldmdb r12!, {d1-d15}
