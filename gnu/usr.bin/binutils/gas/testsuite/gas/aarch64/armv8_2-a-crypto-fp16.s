	# Print a 4 operand instruction
	.macro print_gen4reg op, d, pd1=, pd2=, n, pn1=, pn2=, m, pm1=, pm2=, w	, pw1=, pw2=
	.ifnb \d
		\op \pd1\d\()\pd2, \pn1\n\()\pn2, \pm1\m\()\pm2, \pw1\w\()\pw2
	.else
	.ifnb \n
		\op \pn1\n\()\pn2, \pm1\m\()\pm2, \pw1\w\()\pw2
	.else
		\op \pm1\m\()\pm2, \pw1\w\()\pw2
	.endif
	.endif
	.endm

	.macro gen4reg_iter_d_offset op, d, pd1=, pd2=, r
	.irp m, 03, 82, 13
		\op \pd1\d\()\pd2, [\r, \m]
	.endr
	.endm

	.macro gen4reg_iter_d_n_w op, d, pd1=, pd2=, n, pn1=, pn2=, m, pm1=, pm2=, pw1=, pw2=
	.irp w, 3, 11, 15
		print_gen4reg \op, \d, \pd1, \pd2, \n, \pn1, \pn2, \m, \pm1, \pm2, \w, \pw1, \pw2
	.endr
	.endm

	.macro gen4reg_iter_d_n op, d, pd1=, pd2=, n, pn1=, pn2=, pm1=, pm2=, pw1=, pw2=
	.irp m, 0, 8, 12
		gen4reg_iter_d_n_w \op, \d, \pd1, \pd2, \n, \pn1, \pn2, \m, \pm1, \pm2, \pw1, \pw2
	.endr
	.endm

	.macro gen4reg_iter_d op, d, pd1=, pd2=, pn1=, pn2=, pm1=, pm2=, pw1=, pw2=
	.irp n, 2, 15, 30
		gen4reg_iter_d_n \op, \d, \pd1, \pd2, \n, \pn1, \pn2, \pm1, \pm2, \pw1, \pw2
	.endr
	.endm

	.macro gen4reg_iter op, pd1=, pd2=, pn1=, pn2=, pm1=, pm2=, pw1=, pw2=
	.irp d, 0, 7, 16, 30
		gen4reg_iter_d \op, \d, \pd1, \pd2, \pn1, \pn2, \pm1, \pm2, \pw1, \pw2
	.endr
	.endm

	# Print a 3 operand instruction
	.macro gen3reg_iter op, pd1=, pd2=, pn1=, pn2=, pm1=, pm2=
	.irp d, 0, 7, 16, 30
		gen4reg_iter_d \op,,, \d, \pd1, \pd2, \pn1, \pn2, \pm1, \pm2
	.endr
	.endm

	.macro gen3reg_iter_lane op, pn1=, pn2=, pm1=, pm2=, pw1=, pw2=, x:vararg
	.irp l, \x
		gen4reg_iter_d \op,,,, \pn1, \pn2, \pm1, \pm2, \pw1, \pw2[\l]
	.endr
	.endm

	# Print a 2 operand instruction
	.macro gen2reg_iter op, pd1=, pd2=, pn1=, pn2=
	.irp d, 0, 7, 16, 30
		gen4reg_iter_d_n \op,,,,,, \d, \pd1, \pd2, \pn1, \pn2
	.endr
	.endm

	.macro gen2reg_iter_offset op, pd1=, pd2=, r
	.irp d, 0, 7, 16, 30
		gen4reg_iter_d_offset \op, \d, \pd1, \pd2, \r,
	.endr
	.endm

	# Print a 1 operand instruction
	.macro gen1reg_iter op, pd1=, pd2=
	.irp d, 0, 7, 16, 30
		\op \pd1\d\()\pd2
	.endr
	.endm

	.text
func:
	gen3reg_iter sha512h  q,, q,, v,.2d
	gen3reg_iter sha512h2 q,, q,, v,.2d
	gen2reg_iter sha512su0 v,.2d, v,.2d
	gen3reg_iter sha512su1 v,.2d, v,.2d, v,.2d
	gen4reg_iter eor3 v,.16b, v,.16b, v,.16b, v,.16b
	gen3reg_iter rax1 v,.2d, v,.2d, v,.2d
	gen4reg_iter xar v,.2d, v,.2d, v,.2d,,
	gen4reg_iter bcax v,.16b, v,.16b, v,.16b, v,.16b

	gen4reg_iter sm3ss1 v,.4s, v,.4s, v,.4s, v,.4s
	gen3reg_iter_lane sm3tt1a v,.4s, v,.4s, v,.s, 0, 1, 2, 3
	gen3reg_iter_lane sm3tt1b v,.4s, v,.4s, v,.s, 0, 1, 2, 3
	gen3reg_iter_lane sm3tt2a v,.4s, v,.4s, v,.s, 0, 1, 2, 3
	gen3reg_iter_lane sm3tt2b v,.4s, v,.4s, v,.s, 0, 1, 2, 3
	gen3reg_iter sm3partw1 v,.4s, v,.4s, v,.4s
	gen3reg_iter sm3partw2 v,.4s, v,.4s, v,.4s

	gen2reg_iter sm4e v,.4s, v,.4s
	gen3reg_iter sm4ekey v,.4s, v,.4s, v,.4s

	gen3reg_iter fmlal v,.2s, v,.2h, v,.2h
	gen3reg_iter fmlal v,.4s, v,.4h, v,.4h
	gen3reg_iter fmlsl v,.2s, v,.2h, v,.2h
	gen3reg_iter fmlsl v,.4s, v,.4h, v,.4h

	gen3reg_iter fmlal2 v,.2s, v,.2h, v,.2h
	gen3reg_iter fmlal2 v,.4s, v,.4h, v,.4h
	gen3reg_iter fmlsl2 v,.2s, v,.2h, v,.2h
	gen3reg_iter fmlsl2 v,.4s, v,.4h, v,.4h

	gen3reg_iter_lane fmlal v,.2s, v,.2h, v,.h, 0, 1, 5, 7
	gen3reg_iter_lane fmlal v,.4s, v,.4h, v,.h, 0, 1, 5, 7
	gen3reg_iter_lane fmlsl v,.2s, v,.2h, v,.h, 0, 1, 5, 7
	gen3reg_iter_lane fmlsl v,.4s, v,.4h, v,.h, 0, 1, 5, 7

	gen3reg_iter_lane fmlal2 v,.2s, v,.2h, v,.h, 0, 1, 5, 7
	gen3reg_iter_lane fmlal2 v,.4s, v,.4h, v,.h, 0, 1, 5, 7
	gen3reg_iter_lane fmlsl2 v,.2s, v,.2h, v,.h, 0, 1, 5, 7
	gen3reg_iter_lane fmlsl2 v,.4s, v,.4h, v,.h, 0, 1, 5, 7

