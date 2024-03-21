        .text
        .global _start, dest
_start:
        CALL    dest
        .org    0x20
dest:
        nop
