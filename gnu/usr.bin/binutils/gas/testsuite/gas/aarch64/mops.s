	.arch	armv8.8-a+memtag

dest	.req	x8
src	.req	x11
len	.req	x19
data	.req	x23
zero	.req	xzr

	.macro	pme_seq, op, suffix, r1, r2, r3
	\op\()p\()\suffix \r1, \r2, \r3
	\op\()m\()\suffix \r1, \r2, \r3
	\op\()e\()\suffix \r1, \r2, \r3
	.endm

	.macro	cpy_op1_op2, op, suffix
	pme_seq	\op, \suffix, [x0]!, [x1]!, x30!
	pme_seq	\op, \suffix, [x29]!, [x30]!, x0!
	pme_seq	\op, \suffix, [x30]!, [x0]!, x1!
	pme_seq	\op, \suffix, [dest]!, [src]!, len!
	.endm

	.macro	cpy_op1, op, suffix
	cpy_op1_op2 \op, \suffix
	cpy_op1_op2 \op, \suffix\()rn
	cpy_op1_op2 \op, \suffix\()wn
	cpy_op1_op2 \op, \suffix\()n
	.endm

	.macro	cpy_all, op
	cpy_op1 \op
	cpy_op1 \op, rt
	cpy_op1 \op, wt
	cpy_op1 \op, t
	.endm

	.macro	set_op1_op2, op, suffix
	pme_seq	\op, \suffix, [x0]!, x1!, x30
	pme_seq	\op, \suffix, [x29]!, x30!, x0
	pme_seq	\op, \suffix, [x30]!, x0!, xzr
	pme_seq	\op, \suffix, [dest]!, len!, data
	pme_seq	\op, \suffix, [dest]!, len!, zero
	.endm

	.macro	set_all, op
	set_op1_op2 \op
	set_op1_op2 \op, t
	set_op1_op2 \op, n
	set_op1_op2 \op, tn
	.endm

	cpy_all	cpyf
	cpy_all	cpy

	set_all	set
	set_all	setg

	.arch	armv8.7-a+mops

	cpy_all	cpyf
	cpy_all	cpy

	set_all	set

	.arch	armv8.7-a+mops+memtag

	set_all	setg
