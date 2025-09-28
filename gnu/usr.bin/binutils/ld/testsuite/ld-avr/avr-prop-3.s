        .text
        .global _start, dest
_start:
        CALL    dest
        CALL    dest
        .align  3
dest:
        NOP
