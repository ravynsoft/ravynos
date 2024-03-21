	flh		fa0, 0(a1)
	fsh		fa0, 0(a1)

	fmv.h		fa0, fa1
	fneg.h		fa0, fa1
	fabs.h		fa0, fa1
	fsgnj.h		fa0, fa1, fa2
	fsgnjn.h	fa0, fa1, fa2
	fsgnjx.h	fa0, fa1, fa2

	fadd.h		fa0, fa1, fa2
	fadd.h		fa0, fa1, fa2, rne
	fsub.h		fa0, fa1, fa2
	fsub.h		fa0, fa1, fa2, rne
	fmul.h		fa0, fa1, fa2
	fmul.h		fa0, fa1, fa2, rne
	fdiv.h		fa0, fa1, fa2
	fdiv.h		fa0, fa1, fa2, rne
	fsqrt.h		fa0, fa1
	fsqrt.h		fa0, fa1, rne
	fmin.h		fa0, fa1, fa2
	fmax.h		fa0, fa1, fa2

	fmadd.h		fa0, fa1, fa2, fa3
	fmadd.h		fa0, fa1, fa2, fa3, rne
	fnmadd.h	fa0, fa1, fa2, fa3
	fnmadd.h	fa0, fa1, fa2, fa3, rne
	fmsub.h		fa0, fa1, fa2, fa3
	fmsub.h		fa0, fa1, fa2, fa3, rne
	fnmsub.h	fa0, fa1, fa2, fa3
	fnmsub.h	fa0, fa1, fa2, fa3, rne

	fcvt.w.h	a0, fa1
	fcvt.w.h	a0, fa1, rne
	fcvt.wu.h	a0, fa1
	fcvt.wu.h	a0, fa1, rne
	fcvt.h.w	fa0, a1
	fcvt.h.w	fa0, a1, rne
	fcvt.h.wu	fa0, a1
	fcvt.h.wu	fa0, a1, rne
	fcvt.l.h	a0, fa1
	fcvt.l.h	a0, fa1, rne
	fcvt.lu.h	a0, fa1
	fcvt.lu.h	a0, fa1, rne
	fcvt.h.l	fa0, a1
	fcvt.h.l	fa0, a1, rne
	fcvt.h.lu	fa0, a1
	fcvt.h.lu	fa0, a1, rne

	fmv.x.h		a0, fa1
	fmv.h.x		fa0, a1

	fcvt.s.h	fa0, fa1
	fcvt.d.h	fa0, fa1
	fcvt.q.h	fa0, fa1
	fcvt.h.s	fa0, fa1
	fcvt.h.s	fa0, fa1, rne
	fcvt.h.d	fa0, fa1
	fcvt.h.d	fa0, fa1, rne
	fcvt.h.q	fa0, fa1
	fcvt.h.q	fa0, fa1, rne
	fclass.h	a0, fa1

	feq.h		a0, fa1, fa2
	flt.h		a0, fa1, fa2
	fle.h		a0, fa1, fa2
	fgt.h		a0, fa2, fa1
	fge.h		a0, fa2, fa1
