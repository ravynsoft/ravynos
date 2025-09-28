    .file "pr21404-4.s"
.section	.text,"ax",@progbits
.global nonzero_sym
L1:
    jmp  L1
nonzero_sym:
    nop
    nop
    .p2align 2
.size nonzero_sym, .-nonzero_sym
