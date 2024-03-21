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
        vldm r9, {s1-s31}

        @ VLDM CASE #2
        @ vldm rx!, {...}
        @ -> vldm rx!, {8_words_or_less} for each needed 8_word
        @ This also handles vpop instruction (when rx is sp)
        vldm r6!, {s9-s29}
        @ Explicit VPOP test
        vpop {s1-s9}

        @ VLDM CASE #3
        @ vldmd rx!, {...}
        @ -> vldmb rx!, {8_words_or_less} for each needed 8_word
        vldmdb r11!, {s1-s31}
