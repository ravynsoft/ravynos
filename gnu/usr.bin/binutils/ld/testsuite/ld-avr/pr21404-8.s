    .file "pr21404-8.s"
.section	.text,"ax",@progbits
L1:
    jmp  L1
nonzero_sym:
    nop
    nop
    .p2align 2
.size nonzero_sym, .-nonzero_sym
