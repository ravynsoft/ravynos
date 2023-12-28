.balign 0x1000
.globl _start
_start:
        .skip 0xffc
        adrp x0, _start + 0x80000000
        str x2, [x2]
        mov x2, #0
        ldr x1, [x0, #0x40]
        nop
