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
	gen3reg_iter rmif x,,,,,,
	gen1reg_iter setf8 w,,
	gen1reg_iter setf16 w,,

	gen2reg_iter stlurb w,,[x,]
	gen1reg_iter stlurb w,", [sp]"
	gen3reg_iter stlurb w,, [x,,,]
	gen2reg_iter_offset stlurb w,,sp

	gen2reg_iter ldapurb w,,[x,]
	gen1reg_iter ldapurb w,", [sp]"
	gen3reg_iter ldapurb w,, [x,,,]
	gen2reg_iter_offset ldapurb w,,sp

	gen2reg_iter ldapursb w,,[x,]
	gen1reg_iter ldapursb w,", [sp]"
	gen3reg_iter ldapursb w,, [x,,,]
	gen2reg_iter_offset ldapursb w,,sp

	gen2reg_iter ldapursb x,,[x,]
	gen1reg_iter ldapursb x,", [sp]"
	gen3reg_iter ldapursb x,, [x,,,]
	gen2reg_iter_offset ldapursb x,,sp

	gen2reg_iter stlurh w,,[x,]
	gen1reg_iter stlurh w,", [sp]"
	gen3reg_iter stlurh w,, [x,,,]
	gen2reg_iter_offset stlurh w,,sp

	gen2reg_iter ldapurh w,,[x,]
	gen1reg_iter ldapurh w,", [sp]"
	gen3reg_iter ldapurh w,, [x,,,]
	gen2reg_iter_offset ldapurh w,,sp

	gen2reg_iter ldapursh w,,[x,]
	gen1reg_iter ldapursh w,", [sp]"
	gen3reg_iter ldapursh w,, [x,,,]
	gen2reg_iter_offset ldapursh w,,sp

	gen2reg_iter ldapursh x,,[x,]
	gen1reg_iter ldapursh x,", [sp]"
	gen3reg_iter ldapursh x,, [x,,,]
	gen2reg_iter_offset ldapursh x,,sp

	gen2reg_iter stlur w,,[x,]
	gen1reg_iter stlur w,", [sp]"
	gen3reg_iter stlur w,, [x,,,]
	gen2reg_iter_offset stlur w,,sp

	gen2reg_iter stlur x,,[x,]
	gen1reg_iter stlur x,", [sp]"
	gen3reg_iter stlur x,, [x,,,]
	gen2reg_iter_offset stlur x,,sp

	gen2reg_iter ldapur w,,[x,]
	gen1reg_iter ldapur w,", [sp]"
	gen3reg_iter ldapur w,, [x,,,]
	gen2reg_iter_offset ldapur w,,sp

	gen2reg_iter ldapur x,,[x,]
	gen1reg_iter ldapur x,", [sp]"
	gen3reg_iter ldapur x,, [x,,,]
	gen2reg_iter_offset ldapur x,,sp

	gen2reg_iter ldapursw x,,[x,]
	gen1reg_iter ldapursw x,", [sp]"
	gen3reg_iter ldapursw x,, [x,,,]
	gen2reg_iter_offset ldapursw x,,sp

	cfinv

