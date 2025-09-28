    .file "pr21404-3.s"
.section	.text,"ax",@progbits
.global nonzero_sym
L1:
    jmp  L1
    jmp  L1
    jmp L1
    .p2align 1
nonzero_sym:
    jmp L1
