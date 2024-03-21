target:
	# fli.[sdqh]: Invalid value field
	fli.s	ft1, infinity
	fli.d	ft1, invalid

	# fcvtmod.w.d: Requires explicit rounding mode.
	fcvtmod.w.d	a0, ft1

	# fcvtmod.w.d: Rounding mode other than rtz are reserved.
	fcvtmod.w.d	a0, ft1, rne
	fcvtmod.w.d	a0, ft1, rdn
	fcvtmod.w.d	a0, ft1, rup
	fcvtmod.w.d	a0, ft1, rmm
	fcvtmod.w.d	a0, ft1, dyn
	# fcvtmod.w.d: Invalid rounding mode is invalid.
	fcvtmod.w.d	a0, ft1, invalid
