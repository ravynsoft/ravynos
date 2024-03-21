target:
	fadd.h		a0, a1, a2
	fadd.h		a0, a1, a2, rne
	fsub.h		a0, a1, a2
	fsub.h		a0, a1, a2, rne
	fmul.h		a0, a1, a2
	fmul.h		a0, a1, a2, rne
	fdiv.h		a0, a1, a2
	fdiv.h		a0, a1, a2, rne
	fsqrt.h		a0, a1
	fsqrt.h		a0, a1, rne
	fmin.h		a0, a1, a2
	fmax.h		a0, a1, a2
	fmadd.h		a0, a1, a2, a3
	fmadd.h		a0, a1, a2, a3, rne
	fnmadd.h	a0, a1, a2, a3
	fnmadd.h	a0, a1, a2, a3, rne
	fmsub.h		a0, a1, a2, a3
	fmsub.h		a0, a1, a2, a3, rne
	fnmsub.h	a0, a1, a2, a3
	fnmsub.h	a0, a1, a2, a3, rne

	fcvt.w.h	a0, a1
	fcvt.w.h	a0, a1, rne
	fcvt.wu.h	a0, a1
	fcvt.wu.h	a0, a1, rne
	fcvt.h.w	a0, a1
	fcvt.h.w	a0, a1, rne
	fcvt.h.wu	a0, a1
	fcvt.h.wu	a0, a1, rne
	fcvt.l.h	a0, a1
	fcvt.l.h	a0, a1, rne
	fcvt.lu.h	a0, a1
	fcvt.lu.h	a0, a1, rne
	fcvt.h.l	a0, a1
	fcvt.h.l	a0, a1, rne
	fcvt.h.lu	a0, a1
	fcvt.h.lu	a0, a1, rne

	fcvt.s.h	a0, a2
	fcvt.d.h	a0, a2
	fcvt.q.h	a0, a2
	fcvt.h.s	a0, a2
	fcvt.h.s	a0, a2, rne
	fcvt.h.d	a0, a2
	fcvt.h.d	a0, a2, rne
	fcvt.h.q	a0, a2
	fcvt.h.q	a0, a2, rne

	fsgnj.h		a0, a1, a2
	fsgnjn.h	a0, a1, a2
	fsgnjx.h	a0, a1, a2
	feq.h		a0, a1, a2
	flt.h		a0, a1, a2
	fle.h		a0, a1, a2
	fgt.h		a0, a2, a1
	fge.h		a0, a2, a1
	fmv.h		a0, a1
	fneg.h		a0, a1
	fabs.h		a0, a1
	fclass.h	a0, a1
