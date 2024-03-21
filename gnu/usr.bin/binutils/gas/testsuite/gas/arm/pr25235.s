    .syntax unified
    .thumb

    .align 2
    .type f1, %function
    .thumb_func
    f1:
        nop

    .align 2
    .type f2, %function
    .thumb_func
    f2:
        adr r1, f1
        adr r3, f3
        adr r4, f4


    .align 2
    .type f3, %function
    .thumb_func
    f3:
        nop

    .align 2
    .type f3, %function
    .arm
    f4:
        nop

