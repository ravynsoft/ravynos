        .thumb
        .global f
        .type f, %function
f:
        svc 0xab
        bx lr 

