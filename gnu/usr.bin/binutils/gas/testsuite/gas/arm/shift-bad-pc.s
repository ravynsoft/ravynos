	.syntax unified

	.macro insn4 rd rn rm rs
	  .irp insn, and, eor, sub, rsb, add, adc, sbc, rsc, orr, bic
	    \insn pc,  \rn, \rm, lsr \rs
	    \insn \rd,  pc, \rm, lsr \rs
	    \insn \rd, \rn,  pc, lsr \rs
	    \insn \rd, \rn, \rm, lsr pc
	  .endr
	.endm

	.macro insn3 rn rm rs
	  .irp insn, tst, teq, cmp, cmn, mvn
	    \insn pc,  \rm, lsr \rs
	    \insn \rn,  pc, lsr \rs
	    \insn \rn, \rm, lsr pc
	  .endr
	.endm

	insn4 r0 r1 r2 r3
	insn3 r0 r1 r2
