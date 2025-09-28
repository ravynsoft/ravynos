.text

;;; Use SREG

__start1:
    set

__vec1_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    cpi r18,1
    __gcc_isr 2
    __gcc_isr 0,r16
    clt
__vec1_end:
__data1:
    ldi r16, foo - 2
    .word (__vec1_end - __vec1_start) / 2

;;; Use ZERO

__start2:
    set

__vec2_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    __gcc_isr 2
    reti
    mov r30,r17
    __gcc_isr 2
    reti
    __gcc_isr 0,r16
    clt
__vec2_end:
__data2:
    ldi r16, foo - 1
    .word (__vec2_end - __vec2_start) / 2
