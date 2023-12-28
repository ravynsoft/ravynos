target:
	# fli
	fli.s		ft1, -1
	fli.s		ft1, min
	fli.s		ft1, 0.0000152587890625
	fli.s		ft1, 0x1p-15
	fli.s		ft1, 0.00390625
	fli.s		ft1, 0x1p-7
	fli.s		ft1, 0.0625
	fli.s		ft1, 0x1p-3
	fli.d		ft1, 0.25
	fli.d		ft1, 0x1.4p-2
	fli.d		ft1, 0.375
	fli.d		ft1, 0x1.cp-2
	fli.d		ft1, 0.5
	fli.d		ft1, 0x1.4p-1
	fli.d		ft1, 0.75
	fli.d		ft1, 0x1.cp-1
	fli.q		ft1, 1
	fli.q		ft1, 0x1.4p+0
	fli.q		ft1, 1.5
	fli.q		ft1, 0x1.cp0
	fli.q		ft1, 2.0
	fli.q		ft1, 0x1.4p+1
	fli.q		ft1, 3
	fli.q		ft1, 0x1p2
	fli.h		ft1, 8.0
	fli.h		ft1, 0x1p4
	fli.h		ft1, 128.0
	fli.h		ft1, 0x1p8
	fli.h		ft1, 32768.0
	fli.h		ft1, 0x1p16
	fli.h		ft1, inf
	fli.h		ft1, nan
	# fminm/fmaxm
	fmin.h		ft1, ft2, ft3
	fminm.h		ft1, ft2, ft3
	fmin.s		ft1, ft2, ft3
	fminm.s		ft1, ft2, ft3
	fmin.d		ft1, ft2, ft3
	fminm.d		ft1, ft2, ft3
	fmin.q		ft1, ft2, ft3
	fminm.q		ft1, ft2, ft3
	fmax.h		ft1, ft2, ft3
	fmaxm.h		ft1, ft2, ft3
	fmax.s		ft1, ft2, ft3
	fmaxm.s		ft1, ft2, ft3
	fmax.d		ft1, ft2, ft3
	fmaxm.d		ft1, ft2, ft3
	fmax.q		ft1, ft2, ft3
	fmaxm.q		ft1, ft2, ft3
	# fround/froundnx
	fround.h	fa0, fa1
	fround.h	fa0, fa1, rtz
	fround.s	fa0, fa1
	fround.s	fa0, fa1, rtz
	fround.d	fa0, fa1
	fround.d	fa0, fa1, rtz
	fround.q	fa0, fa1
	fround.q	fa0, fa1, rtz
	froundnx.h	fa0, fa1
	froundnx.h	fa0, fa1, rtz
	froundnx.s	fa0, fa1
	froundnx.s	fa0, fa1, rtz
	froundnx.d	fa0, fa1
	froundnx.d	fa0, fa1, rtz
	froundnx.q	fa0, fa1
	froundnx.q	fa0, fa1, rtz
	# fcvtmod.w.d
	fcvtmod.w.d	a0, ft1, rtz
	# fltq/fleq
	flt.h		a0, ft1, ft2
	fltq.h		a0, ft1, ft2
	flt.s		a0, ft1, ft2
	fltq.s		a0, ft1, ft2
	flt.d		a0, ft1, ft2
	fltq.d		a0, ft1, ft2
	flt.q		a0, ft1, ft2
	fltq.q		a0, ft1, ft2
	fle.h		a0, ft1, ft2
	fleq.h		a0, ft1, ft2
	fle.s		a0, ft1, ft2
	fleq.s		a0, ft1, ft2
	fle.d		a0, ft1, ft2
	fleq.d		a0, ft1, ft2
	fle.q		a0, ft1, ft2
	fleq.q		a0, ft1, ft2
