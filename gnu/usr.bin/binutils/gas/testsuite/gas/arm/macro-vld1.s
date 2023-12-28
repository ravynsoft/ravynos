	.fpu neon
        .macro sfi_breg basereg, insn, operands:vararg
                .macro _sfi_breg_doit B
                \insn \operands
                .endm
                _sfi_breg_doit \basereg
                .purgem _sfi_breg_doit
        .endm
	sfi_breg r0, vld1.8 {d0}, [\B]
	sfi_breg r0, vld1.8 { d0 }, [\B]
