.text
	.macro gen_illegal op, p1, p2, p3
	.irp w, v16.\p3, v27.\p3, v31.\p3
		\op v2.\p1, v12.\p2, \w[0]
	.endr
	.endm

	.macro gen_illegal2 op, p1, p2, p3
	.irp x, \p1\()2
	.irp y, \p2\()12
	.irp w, v16.\p3, v27.\p3, v31.\p3
		\op \x, \y, \w[0]
	.endr
	.endr
	.endr
	.endm

	gen_illegal fmla, 4h, 4h, h
	gen_illegal fmlal, 4s, 4h, h
	gen_illegal fmlal2, 4s, 4h, h
	gen_illegal fmls, 4h, 4h, h
	gen_illegal fmlsl, 4s, 4h, h
	gen_illegal fmlsl2, 4s, 4h, h
	gen_illegal fmul, 4h, 4h, h
	gen_illegal fmulx, 4h, 4h, h
	gen_illegal mla, 4h, 4h, h
	gen_illegal mls, 4h, 4h, h
	gen_illegal mul, 4h, 4h, h
	gen_illegal smlal, 4s, 4h, h
	gen_illegal smlal2, 4s, 8h, h
	gen_illegal smlsl, 4s, 4h, h
	gen_illegal smlsl2, 4s, 8h, h
	gen_illegal smull, 4s, 4h, h
	gen_illegal smull2, 4s, 8h, h
	gen_illegal sqdmlal, 4s, 4h, h
	gen_illegal sqdmlal2, 4s, 8h, h
	gen_illegal sqdmlsl, 4s, 4h, h
	gen_illegal sqdmlsl2, 4s, 8h, h
	gen_illegal sqdmulh, 4h, 4h, h
	gen_illegal sqdmull, 4s, 4h, h
	gen_illegal sqdmull2, 4s, 8h, h
	gen_illegal sqrdmlah, 4h, 4h, h
	gen_illegal sqrdmlsh, 4h, 4h, h
	gen_illegal sqrdmulh, 4h, 4h, h
	gen_illegal umlal, 4s, 4h, h
	gen_illegal umlal2, 4s, 8h, h
	gen_illegal umlsl, 4s, 4h, h
	gen_illegal umlsl2, 4s, 8h, h
	gen_illegal umull, 4s, 4h, h
	gen_illegal umull2, 4s, 8h, h

	gen_illegal2 sqdmlal, s, h, h
	gen_illegal2 sqdmlsl, s, h, h
	gen_illegal2 sqdmull, s, h, h
	gen_illegal2 sqdmulh, h, h, h
	gen_illegal2 sqrdmulh, h, h, h
	gen_illegal2 fmla, h, h, h
	gen_illegal2 fmls, h, h, h
	gen_illegal2 fmul, h, h, h
	gen_illegal2 fmulx, h, h, h
	gen_illegal2 sqrdmlah, h, h, h
	gen_illegal2 sqrdmlsh, h, h, h
