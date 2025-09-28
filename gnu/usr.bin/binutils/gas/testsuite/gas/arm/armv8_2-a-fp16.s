	.macro fmac_iter_d_n op, d, n
	.irp m, 0, 8, 15
		vfmal.f16 d\d, s\n, s\m
		vfmal.f16 d\n, s\d, s\m[0]
		vfmsl.f16 d\d, s\n, s\m
		vfmsl.f16 d\n, s\d, s\m[1]
	.endr
	.endm

	.macro fmac_iter_d op, d
	.irp n, 2, 15, 30
		fmac_iter_d_n \op, \d, \n
	.endr
	.endm

	.macro iter
	.irp d, 0, 7, 16, 31
		fmac_iter_d \op, \d
	.endr
	.endm

	.macro fmac_q_iter_d_n op, d, n
	.irp m, 0, 7
		vfmal.f16 q\n, d\d, d\m
		vfmal.f16 q\d, d\n, d\m[0]
		vfmsl.f16 q\n, d\d, d\m
		vfmsl.f16 q\d, d\n, d\m[3]
	.endr
	.endm

	.macro fmac_q_iter_d op, d
	.irp n, 2, 3, 13
		fmac_q_iter_d_n \op, \d, \n
	.endr
	.endm

	.macro q_iter
	.irp d, 0, 1, 6, 15
		fmac_q_iter_d \op, \d
	.endr
	.endm

	.text
func:
	iter
	q_iter
