
    .text
    .align 4
main:
    mov %r0, 1
    mov %r1, 1
    mov %r2, 2
    lddw %r6, bar
    call %r6
    exit

bar:
    mov %r0, 0
    exit
