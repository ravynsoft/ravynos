    .file "pr21404-7.s"
.section	.text,"ax",@progbits
L1:
    jmp  L1
    jmp  L1
    jmp L1
    .p2align 1
nonzero_sym:
    jmp L1
