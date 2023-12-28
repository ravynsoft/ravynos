.text
.globl  _start
_start:
    # ld.bu FMT14
    ld.bu 0x0[r10], r20
    ld.bu 0x4[r10], r20
    ld.bu 0x8[r10], r20

    ld.b  0x0[r10], r20
    ld.b  0x4[r10], r20
    ld.b  0x8[r10], r20

exit:
    mov r0, r1

.data
data:
