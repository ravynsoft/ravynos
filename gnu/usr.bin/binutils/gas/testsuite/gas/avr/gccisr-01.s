.text

;;; Use SREG

__start1:
    set

__vec1_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    cpi r16,1
    __gcc_isr 2
    __gcc_isr 0,r0
    clt
__vec1_end:
__data1:
    ldi r16, foo - 2
    .word (__vec1_end - __vec1_start) / 2

;;; Nothing used.

__start2:
    set

__vec2_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    ldi r30, 1
    lds r31, 0
    sts 0, r31
    movw r2, r4
    swap r17
    __gcc_isr 2
    reti
    __gcc_isr 2
    cpse r7, r8
    sei
    cli
    in  r10, 0x3f
    out 0x3f, r10
    reti
    __gcc_isr 0,r0
    clt
__vec2_end:
__data2:
    ldi r16, foo - 0
    .word (__vec2_end - __vec2_start) / 2

;;; Use SREG, ZERO and R24

__start3:
    set

__vec3_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    __gcc_isr 2
    reti
    __gcc_isr 2
    reti
    inc r1
    __gcc_isr 0,r24
    clt
__vec3_end:
__data3:
    ldi r16, foo - 3
    .word (__vec3_end - __vec3_start) / 2

;;; Use SREG, ZERO, TMP and R24

__start4:
    set

__vec4_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    __gcc_isr 2
    reti
    __gcc_isr 2
    reti
    mul 16, 17
    __gcc_isr 0,r24
    clt
__vec4_end:
__data4:
    ldi r16, foo - 4
    .word (__vec4_end - __vec4_start) / 2

;;; Use TMP

__start5:
    set

__vec5_start:
    __gcc_isr 1
    lpm
    foo = __gcc_isr.n_pushed
    __gcc_isr 2
    reti
    __gcc_isr 2
    reti
    __gcc_isr 0,r0
    clt
__vec5_end:
__data5:
    ldi r16, foo - 1
    .word (__vec5_end - __vec5_start) / 2

;;; Use SREG, R26

__start6:
    set

__vec6_start:
    __gcc_isr 1
    foo = __gcc_isr.n_pushed
    __gcc_isr 2
    reti
    __gcc_isr 2
    reti
    clc
    __gcc_isr 0,r26
    clt
__vec6_end:
__data6:
    ldi r16, foo - 2
    .word (__vec6_end - __vec6_start) / 2
