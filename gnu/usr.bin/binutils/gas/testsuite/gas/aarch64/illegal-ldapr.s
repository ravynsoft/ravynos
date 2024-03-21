/* Test illegal ARMv8.3 weaker release consistency load instructions.  */

	/* <mnemonic>	<Wt>, [<Xn|SP>{,#0}] */
	.macro LR32 op
	\op	w1, [xz]
	\op	w1, [x7, #8]
	\op	w1, [x7, #8]!
	\op	w1, [x7], #8
	.endm

.text
	/* Good.  */
	ldaprb w0, [x1]
	ldaprh w0, [x1]
	ldapr x0, [x1]

	/* Bad.  */
	ldaprb x0, [x1]
	ldaprh x0, [x1]
	ldapr x0, [x1,#8]

	.irp	op, ldaprb, ldaprh, ldapr
	LR32	\op
	.endr

