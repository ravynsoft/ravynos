target:
	fadd.s	a0, a1, a2
	fadd.s	a0, a1, a2, rne
	fsub.s	a0, a1, a2
	fsub.s	a0, a1, a2, rne
	fmul.s	a0, a1, a2
	fmul.s	a0, a1, a2, rne
	fdiv.s	a0, a1, a2
	fdiv.s	a0, a1, a2, rne
	fsqrt.s	a0, a1
	fsqrt.s	a0, a1, rne
	fmin.s	a0, a1, a2
	fmax.s	a0, a1, a2
	fmadd.s		a0, a1, a2, a3
	fmadd.s		a0, a1, a2, a3, rne
	fnmadd.s	a0, a1, a2, a3
	fnmadd.s	a0, a1, a2, a3, rne
	fmsub.s		a0, a1, a2, a3
	fmsub.s		a0, a1, a2, a3, rne
	fnmsub.s	a0, a1, a2, a3
	fnmsub.s	a0, a1, a2, a3, rne

	fcvt.w.s	a0, a1
	fcvt.w.s	a0, a1, rne
	fcvt.wu.s	a0, a1
	fcvt.wu.s	a0, a1, rne
	fcvt.l.s	a0, a1
	fcvt.l.s	a0, a1, rne
	fcvt.lu.s	a0, a1
	fcvt.lu.s	a0, a1, rne
	fcvt.s.w	a0, a1
	fcvt.s.w	a0, a1, rne
	fcvt.s.wu	a0, a1
	fcvt.s.wu	a0, a1, rne
	fcvt.s.l	a0, a1
	fcvt.s.l	a0, a1, rne
	fcvt.s.lu	a0, a1
	fcvt.s.lu	a0, a1, rne

	fsgnj.s		a0, a1, a2
	fsgnjn.s	a0, a1, a2
	fsgnjx.s	a0, a1, a2
	feq.s		a0, a1, a2
	flt.s		a0, a1, a2
	fle.s		a0, a1, a2
	fgt.s		a0, a1, a2
	fge.s		a0, a1, a2
	fmv.s		a0, a1
	fneg.s		a0, a1
	fabs.s		a0, a1
	fclass.s	a0, a1
