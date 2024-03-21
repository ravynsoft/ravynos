        .section ".text.1", "ax"
        .global _start
_start:
        .org 0x20
        nop
        .org 0x44,5
        nop


        .section ".text.2", "ax"
        .global test2
text2:
        .org 0x20
        nop
        .align 4
        nop
        .align 4,3
        nop
        .org 0x200
        nop

        .section ".text.3", "ax"
        .global test3
text3:
        .org 0x0
        nop
        nop
        .align 8
        nop
