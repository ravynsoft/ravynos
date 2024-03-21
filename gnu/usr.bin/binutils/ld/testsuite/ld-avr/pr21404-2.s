    .file "pr21404.s"
.section	.text,"ax",@progbits
.global size_before_align
.global size_after_align
.global main
.global nonzero_sym_before_align
.global nonzero_sym_after_align
.global nonzero_sym_after_end
main:
size_before_align:
size_after_align:
L1:
    jmp  L1
nonzero_sym_before_align:
nonzero_sym_after_align:
nonzero_sym_after_end:
    jmp  L1
.size size_before_align, .-size_before_align
.size nonzero_sym_before_align, .-nonzero_sym_before_align
    .p2align 1
.size size_after_align, .-size_after_align
.size nonzero_sym_after_align, .-nonzero_sym_after_align
.word L1
.size main, .-main
.size nonzero_sym_after_end, .-nonzero_sym_after_end
