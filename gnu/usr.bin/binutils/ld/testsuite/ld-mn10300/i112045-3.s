       .text
L001:
        mov     L001,A0
L002:
        mov     L001,A0
L003:

        .section        .rodata
L004:
        .long   L003-L001
        .long   L003-L002
