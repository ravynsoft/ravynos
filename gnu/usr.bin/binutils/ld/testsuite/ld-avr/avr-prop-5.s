        .text
        .global _start, dest
_start:
        CALL    dest
        .align  1
dest:
        NOP
