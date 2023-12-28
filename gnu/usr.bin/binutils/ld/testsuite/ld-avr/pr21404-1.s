    .file "pr21404-1.s"
.section	.text,"ax",@progbits
.global nonzero_sym
.global main
main:
L1:
    jmp  L1
nonzero_sym:
    nop
.size main, .-main
.size nonzero_sym, .-nonzero_sym
