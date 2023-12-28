	.macro m arg1, arg2
	.byte \arg1
	.byte \arg2
	.endm

	m 1, 2
	m 3 4
	m "5", "6"
	m "7" "8"
	m "" "" ""

	.if 0
	m 1 +2
	m (3) +4
	m (5) (6)
	.endif
