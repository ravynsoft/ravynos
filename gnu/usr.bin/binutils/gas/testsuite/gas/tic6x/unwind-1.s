.cfi_sections .c6xabi.exidx

# standard layout
.p2align 8
f0:
.cfi_startproc
stw .d2t2 B3, *B15--(16)
.cfi_def_cfa_offset 16
.cfi_offset 19, 0
stw .d2t1 A11, *+B15(12)
.cfi_offset 11, -4
nop 4
.cfi_endproc
.endp

# standard layout (pr0)
.p2align 8
f1:
.cfi_startproc
.cfi_def_cfa_offset 8
stw .d2t1 A11, *+B15(8)
.cfi_offset 11, -0
stw .d2t1 A10, *+B15(4)
.cfi_offset 10, -4
nop 4
.cfi_endproc
.personalityindex 0
.endp

# standard layout (pr1)
.p2align 8
f2:
.cfi_startproc
stw .d2t2 B15, *B15--(24)
.cfi_def_cfa_offset 24
.cfi_offset 31, 0
stw .d2t2 B10, *+B15(20)
.cfi_offset 26, -4
stw .d2t2 B3, *+B15(16)
.cfi_offset 19, -8
stdw .d2t1 A11:A10, *+B15(8)
.cfi_offset 11, -12
.cfi_offset 10, -16
nop 4
.cfi_endproc
.personalityindex 1
.endp

# standard layout (pr3)
.p2align 8
f3:
.cfi_startproc
stw .d2t2 B3, *B15--(16)
.cfi_def_cfa_offset 16
.cfi_offset 19, 0
stw .d2t1 A11, *+B15(12)
.cfi_offset 11, -4
nop 4
.cfi_endproc
.personalityindex 3
.endp

# compact layout
.p2align 8
f4:
.cfi_startproc
stw .d2t2 B3, *B15--(8)
.cfi_offset 19, 0
.cfi_def_cfa_offset 8
stw .d2t1 A11, *B15--(8)
.cfi_offset 11, -8
.cfi_def_cfa_offset 16
nop 4
.cfi_endproc
.endp

# compact layout (pr0)
.p2align 8
f5:
.cfi_startproc
stw .d2t2 B3, *B15--(8)
.cfi_offset 19, 0
.cfi_def_cfa_offset 8
stw .d2t1 A11, *B15--(8)
.cfi_offset 11, -8
.cfi_def_cfa_offset 16
nop 4
.cfi_endproc
.personalityindex 0
.endp

# compact layout (pr4)
.p2align 8
f6:
.cfi_startproc
stw .d2t2 B3, *B15--(8)
.cfi_offset 19, 0
.cfi_def_cfa_offset 8
stw .d2t1 A11, *B15--(8)
.cfi_offset 11, -8
.cfi_def_cfa_offset 16
nop 4
.cfi_endproc
.personalityindex 4
.endp

# compact layout (aligned pair)
.p2align 8
f7:
.cfi_startproc
stw .d2t2 B10, *B15--(8)
.cfi_offset 26, 0
.cfi_def_cfa_offset 8
stw .d2t2 B3, *B15--(8)
.cfi_offset 19, -8
.cfi_def_cfa_offset 8
stdw .d2t1 A11:A10, *B15--(8)
.cfi_offset 11, -12
.cfi_offset 10, -16
.cfi_def_cfa_offset 24
nop 4
.cfi_endproc
.endp

# compact layout (aligned pair + 1)
.p2align 8
f8:
.cfi_startproc
stw .d2t2 B3, *B15--(8)
.cfi_offset 19, 0
.cfi_def_cfa_offset 8
stdw .d2t1 A13:A12, *B15--(8)
.cfi_offset 13, -4
.cfi_offset 12, -8
.cfi_def_cfa_offset 16
stw .d2t1 A10, *B15--(8)
.cfi_offset 10, -16
.cfi_def_cfa_offset 24
nop 4
.cfi_endproc
.endp

# compact layout (misaligned pair)
.p2align 8
f9:
.cfi_startproc
stw .d2t2 B11, *B15--(8)
.cfi_offset 27, 0
.cfi_def_cfa_offset 8
stw .d2t2 B10, *B15--(8)
.cfi_offset 26, -8
.cfi_def_cfa_offset 16
nop 4
.cfi_endproc
.endp

# standard frame pointer
.p2align 8
fa:
.cfi_startproc
stw .d2t1 A15, *B15--(16)
.cfi_def_cfa_offset 8
.cfi_offset 15, 0
mv .s1x B15, A15
addk .s1 16, A15
.cfi_def_cfa 15, 0
stw .d2t1 A11, *+B15(12)
.cfi_offset 11, -4
nop 4
.cfi_endproc
.endp

# compact frame pointer
.p2align 8
fb:
.cfi_startproc
stw .d2t1 A15, *B15--(8)
.cfi_def_cfa_offset 8
.cfi_offset 15, 0
mv .s1x B15, A15
addk .s1 16, A15
.cfi_def_cfa 15, 0
stw .d2t1 A11, *B15--(8)
.cfi_offset 11, -8
nop 4
.cfi_endproc
.endp

# custom layout
.p2align 8
fc:
.cfi_startproc
sub .s2 B15, 16, B15
stw .d2t2 B3, *+B15(12)
.cfi_def_cfa_offset 16
.cfi_offset 19, -4
nop 4
.cfi_endproc
.endp

# custom layout
.p2align 8
fd:
.cfi_startproc
sub .s2 B15, 16, B15
stw .d2t2 B3, *+B15(12)
.cfi_def_cfa_offset 16
.cfi_offset 19, -4
stw .d2t1 A11, *+B15(8)
.cfi_offset 11, -8
nop 4
.cfi_endproc
.endp

# custom layout
.p2align 8
fe:
.cfi_startproc
sub .s2 B15, 16, B15
stw .d2t2 B3, *+B15(12)
.cfi_def_cfa_offset 16
.cfi_offset 19, -4
stw .d2t1 A11, *+B15(4)
.cfi_offset 11, -12
nop 4
.cfi_endproc
.endp

# custom layout
.p2align 8
ff:
.cfi_startproc
addk .s2 -24, B15
stw .d2t2 B3, *+B15(24)
.cfi_def_cfa_offset 24
.cfi_offset 19, 0
stw .d2t1 A11, *+B15(4)
.cfi_offset 11, -20
nop 4
.cfi_endproc
.endp

