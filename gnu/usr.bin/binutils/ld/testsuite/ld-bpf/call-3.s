    .text
    .align 4
    .global bar
bar:
    mov %r0, 0
    exit

main:
    mov %r0, 3
    mov %r1, 1
    call bar
    exit
