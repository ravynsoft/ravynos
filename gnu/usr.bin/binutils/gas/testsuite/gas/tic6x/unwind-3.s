.cfi_sections .c6xabi.exidx

.text
# standard layout
.p2align 8
foo:
.cfi_startproc
.personalityindex 0
stw .d2t2 B3, *B15--(16)
.cfi_def_cfa_offset 16
.cfi_offset B3, 0


.section .text.bar, "ax"

bar:
.cfi_startproc
stw .d2t2 B13, *B15--(16)
.cfi_def_cfa_offset 64
.cfi_offset B13, 0
stw .d2t2 B13, *+B15(12)
.cfi_offset B11, -4
nop 4
.cfi_endproc
.endp

.text

stw .d2t1 A10, *+B15(12)
.cfi_offset A10, -4
nop 4
.cfi_endproc
.endp
