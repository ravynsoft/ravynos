.cfi_sections .c6xabi.exidx

.cfi_startproc
# stack pointer offset too large for personality routine
.cfi_def_cfa_offset 0x3f8
.cfi_endproc
.personalityindex 3
.endp

.cfi_startproc
.cfi_def_cfa_offset 8
stw .d2t1 A11, *+B15(8)
.cfi_offset 11, -0
stw .d2t1 A10, *+B15(4)
.cfi_offset 10, -4
nop 4
.cfi_endproc
# stack frame layout does not match personality routine
.personalityindex 4
.endp

.cfi_startproc
stw .d2t2 B3, *B15--(8)
.cfi_offset 19, 0
.cfi_def_cfa_offset 8
stw .d2t1 A11, *B15--(8)
.cfi_offset 11, -8
.cfi_def_cfa_offset 16
nop 4
.cfi_endproc
# stack frame layout does not match personality routine
.personalityindex 3
.endp

.cfi_startproc
stw .d2t2 B4, *B15--(8)
# unable to generate unwinding opcode for reg 20
.cfi_offset 20, 0
.cfi_endproc
.endp

.cfi_startproc
mv .s2 B3, B4
# unable to generate unwinding opcode for reg 20
.cfi_register 19, 20
.cfi_endproc
.endp

.cfi_startproc
mv .s2 B4, B3
# unable to generate unwinding opcode for reg 20
.cfi_register 20, 19
.cfi_endproc
.endp

.cfi_startproc
stw .d2t2 B10, *B15--(8)
# unable to generate unwinding opcode for reg 20
.cfi_offset 26, 0
mv .s2 B3, B10
# unable to restore return address from previously restored reg
.cfi_register 19, 26
.cfi_endproc
.endp

.cfi_startproc
nop
# unhandled CFA insn for unwinding (259)
.cfi_escape 42
.cfi_endproc
.endp

.cfi_startproc
nop
# unable to generate unwinding opcode for frame pointer reg 14
.cfi_def_cfa_register 14
.cfi_endproc
.endp

.cfi_startproc
nop
# unable to generate unwinding opcode for frame pointer offset
.cfi_def_cfa 15, 8
.cfi_endproc
.endp

.cfi_startproc
nop
# unwound stack pointer not doubleword aligned
.cfi_def_cfa_offset 12
.cfi_endproc
.endp

.cfi_startproc
nop
.cfi_offset 10, 0
# stack frame layout too complex for unwinder
.cfi_offset 11, -0x808
.cfi_def_cfa_offset 0x10000
.cfi_endproc
.endp

.cfi_startproc
nop
.cfi_offset 12, -0
.cfi_offset 11, -4
.cfi_offset 10, -8
.cfi_def_cfa_offset 8
# unwound frame has negative size
.cfi_endproc
.endp


