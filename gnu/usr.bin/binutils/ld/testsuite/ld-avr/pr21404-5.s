    .file "pr21404-1.s"
.section	.text,"ax",@progbits
_main:
L1:
    jmp  L1
_nonzero_sym:
    nop
.size _main, .-_main
.size _nonzero_sym, .-_nonzero_sym
