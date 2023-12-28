target:
	fadd.q	a0, a2, a4
	fadd.q	a0, a2, a4, rne
	fsub.q	a0, a2, a4
	fsub.q	a0, a2, a4, rne
	fmul.q	a0, a2, a4
	fmul.q	a0, a2, a4, rne
	fdiv.q	a0, a2, a4
	fdiv.q	a0, a2, a4, rne
	fsqrt.q	a0, a2
	fsqrt.q	a0, a2, rne
	fmin.q	a0, a2, a4
	fmax.q	a0, a2, a4
	fmadd.q		a0, a2, a4, a6
	fmadd.q		a0, a2, a4, a6, rne
	fnmadd.q	a0, a2, a4, a6
	fnmadd.q	a0, a2, a4, a6, rne
	fmsub.q		a0, a2, a4, a6
	fmsub.q		a0, a2, a4, a6, rne
	fnmsub.q	a0, a2, a4, a6
	fnmsub.q	a0, a2, a4, a6, rne

	fcvt.w.q	a0, a2
	fcvt.w.q	a0, a2, rne
	fcvt.wu.q	a0, a2
	fcvt.wu.q	a0, a2, rne
	fcvt.l.q	a0, a2
	fcvt.l.q	a0, a2, rne
	fcvt.lu.q	a0, a2
	fcvt.lu.q	a0, a2, rne
	fcvt.q.w	a0, a2
	fcvt.q.wu	a0, a2
	fcvt.q.l	a0, a2
	fcvt.q.lu	a0, a2

	fcvt.q.s	a0, a2
	fcvt.q.d	a0, a2
	fcvt.s.q	a0, a2
	fcvt.s.q	a0, a2, rne
	fcvt.d.q	a0, a2
	fcvt.d.q	a0, a2, rne

	fsgnj.q		a0, a2, a4
	fsgnjn.q	a0, a2, a4
	fsgnjx.q	a0, a2, a4
	feq.q		a0, a2, a4
	flt.q		a0, a2, a4
	fle.q		a0, a2, a4
	fgt.q		a0, a2, a4
	fge.q		a0, a2, a4
	fmv.q		a0, a2
	fneg.q		a0, a2
	fabs.q		a0, a2
	fclass.q	a0, a2
