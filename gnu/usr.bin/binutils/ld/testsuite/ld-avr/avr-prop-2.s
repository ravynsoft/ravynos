        .text
        .global _start, label1, label2, dest
_start:
        CALL    dest
        .org    0x10
label1:
        CALL    dest
        .org    0x20
label2:
        CALL    dest
        .org    0x30
dest:
        nop
