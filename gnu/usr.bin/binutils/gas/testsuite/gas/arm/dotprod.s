	.macro dotprod_iter_d_n op, d, n
	.irp m, 0, 8, 15
		\op d\d, d\n, d\m
		\op d\d, d\n, d\m[0]
	.endr
	.endm

	.macro dotprod_iter_d op, d
	.irp n, 2, 15, 30
		dotprod_iter_d_n \op, \d, \n
	.endr
	.endm

	.macro dotprod_iter op
	.irp d, 0, 7, 16, 31
		dotprod_iter_d \op, \d
	.endr
	.endm

	.macro dotprod_q_iter_d_n op, d, n
	.irp m, 0, 7, 15
		\op q\d, q\n, q\m
		\op q\d, q\n, d\m[1]
	.endr
	.endm

	.macro dotprod_q_iter_d op, d
	.irp n, 2, 3, 14
		dotprod_q_iter_d_n \op, \d, \n
	.endr
	.endm

	.macro dotprod_q_iter op
	.irp d, 0, 1, 6, 13
		dotprod_q_iter_d \op, \d
	.endr
	.endm

	.text
func:
	dotprod_iter vudot.u8
	dotprod_iter vsdot.s8
	dotprod_q_iter vudot.u8
	dotprod_q_iter vsdot.s8
