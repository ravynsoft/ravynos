	/* ARMv8.3 weaker release consistency load instructions.  */

	/* <mnemonic>	<Wt>, [<Xn|SP>{,#0}] */
	.macro LR32 op
	\op	w1, [x7]
	\op	w1, [x7, #0]
	\op	w1, [x7, 0]
	.endm

	/* <mnemonic>	<Xt>, [<Xn|SP>{,#0}] */
	.macro LR64 op
	\op	x1, [x7]
	\op	x1, [x7, #0]
	\op	x1, [x7, 0]
	.endm

func:
	.irp	op, ldaprb, ldaprh, ldapr
	LR32	\op
	.endr

	LR64	ldapr
