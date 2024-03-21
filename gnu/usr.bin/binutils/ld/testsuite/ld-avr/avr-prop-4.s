        .text
        .global _start, dest
_start:
        CALL    dest
        CALL    dest
        .align  2
dest:
        NOP
