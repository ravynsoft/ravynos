	.if	0
	.if	1
	.endc
	.long	0
	.if	0
	.long	1
	.endc
	.else
	.if	1
	.endc
	.long	2
	.if	0
	.long	3
	.else
	.long	4
	.endc
	.endc

	.if	0
	.long	5
	.elseif	1
	.if	0
	.long	6
	.elseif	1
	.long	7
	.endif
	.elseif	1
	.long	8
	.else
	.long	9
	.endif

	.comm	v_c, 1
	.ifndef v_c
	.err
	.endif

	.if	x <> x
	.err
	.endif
	.equiv	y, x
	.ifndef	y
	.err
	.endif
	.if	x <> y
	.err
	.endif
	.equiv	z, x
	.if	y <> z
	.err
	.endif

	.equiv	v_a, y + 1
	.equiv	v_b, z - 1
	.if	v_a == x
	.err
	.endif
	.if	v_a - 1 <> x
	.err
	.endif
	.if	v_a <> v_b + 2
	.err
	.endif
	.if	v_a - v_b <> 2
	.err
	.endif

	.equiv	x, 0
	.if	y
	.err
	.elseif	y
	.err
	.endif

	.macro	m x, y
	.ifb \x
	.long	-1
	.else
	.long	\x
	.endif
	.ifnb \y
	.long	\y
	.else
	.long	-1
	.endif
	.endm
	m	,
	m	, 10
	m	11,
	m	12, 13

	.if 0
#define x "m" \
	(x)
#define y \
	"m" \
	(y)
#define z \
	((z) \
	 + 1)
	.endif

	.p2align 5,0
