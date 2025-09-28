	.set	mips16

	.macro	mem9pos op, ri, base
	\op	\ri,0(\base)
	\op	\ri,1(\base)
	\op	\ri,2(\base)
	\op	\ri,3(\base)
	\op	\ri,4(\base)
	\op	\ri,8(\base)
	\op	\ri,16(\base)
	\op	\ri,32(\base)
	\op	\ri,64(\base)
	\op	\ri,128(\base)
	\op	\ri,255(\base)
	.endm

	.macro	mem9neg op, ri, base
	\op	\ri,-1(\base)
	\op	\ri,-2(\base)
	\op	\ri,-3(\base)
	\op	\ri,-4(\base)
	\op	\ri,-8(\base)
	\op	\ri,-16(\base)
	\op	\ri,-32(\base)
	\op	\ri,-64(\base)
	\op	\ri,-128(\base)
	\op	\ri,-256(\base)
	.endm

	.macro	mem9 op, ri, base
	mem9pos	\op, \ri, \base
	mem9neg	\op, \ri, \base
	.endm

	.macro	mem op, ri, base
	mem9pos	\op, \ri, \base
	\op	\ri,256(\base)
	\op	\ri,512(\base)
	\op	\ri,1024(\base)
	\op	\ri,2048(\base)
	\op	\ri,4096(\base)
	\op	\ri,8192(\base)
	\op	\ri,16384(\base)
	\op	\ri,32767(\base)
	mem9neg	\op, \ri, \base
	\op	\ri,-512(\base)
	\op	\ri,-1024(\base)
	\op	\ri,-2048(\base)
	\op	\ri,-4096(\base)
	\op	\ri,-8192(\base)
	\op	\ri,-16384(\base)
	\op	\ri,-32768(\base)
	.endm

	.macro	alupos op, args:vararg
	\op	\args, 0
	\op	\args, 1
	\op	\args, 2
	\op	\args, 4
	\op	\args, 8
	\op	\args, 16
	\op	\args, 32
	\op	\args, 64
	\op	\args, 128
	\op	\args, 256
	\op	\args, 512
	\op	\args, 1024
	\op	\args, 2048
	\op	\args, 4096
	\op	\args, 8192
	\op	\args, 16384
	\op	\args, 32767
	.endm

	.macro	aluneg op, args:vararg
	\op	\args, -1
	\op	\args, -2
	\op	\args, -4
	\op	\args, -8
	\op	\args, -16
	\op	\args, -32
	\op	\args, -64
	\op	\args, -128
	\op	\args, -256
	\op	\args, -512
	\op	\args, -1024
	\op	\args, -2048
	\op	\args, -4096
	\op	\args, -8192
	\op	\args, -16384
	\op	\args, -32768
	.endm

	.macro	aluu op, args:vararg
	alupos	\op, \args
	\op	\args, 32768
	\op	\args, 65535
	.endm

	.macro	alu op, args:vararg
	alupos	\op, \args
	aluneg	\op, \args
	.endm

	.macro	bit op, ry, rx
	\op	\ry, \rx, 0, 32
	\op	\ry, \rx, 1, 25
	\op	\ry, \rx, 2, 17
	\op	\ry, \rx, 3, 13
	\op	\ry, \rx, 4, 9
	\op	\ry, \rx, 6, 7
	\op	\ry, \rx, 8, 5
	\op	\ry, \rx, 12, 4
	\op	\ry, \rx, 16, 3
	\op	\ry, \rx, 24, 2
	\op	\ry, \rx, 31, 1
	.endm

foo:
	mem	lw, $2, $gp
	mem	lh, $2, $gp
	mem	lhu, $2, $gp
	mem	lb, $2, $gp
	mem	lbu, $2, $gp
	mem	sw, $2, $gp
	mem	sh, $2, $gp
	mem	sb, $2, $gp

	mem9	ll, $2, $3
	mem9	lwl, $2, $3
	mem9	lwr, $2, $3
	mem9	sc, $2, $3
	mem9	swl, $2, $3
	mem9	swr, $2, $3
	mem9	cache, 2, $3
	mem9	cache, 29, $3
	mem9	pref, 8, $3
	mem9	pref, 23, $3

	alu	addiu, $2, $gp
	alu	addu, $2, $gp
	aluu	lui, $2
	aluu	andi, $2
	aluu	ori, $2
	aluu	xori, $2

	bit	ext, $2, $3
	bit	ins, $2, $3
	bit	ins, $6, $0

	movn	$2, $3, $4
	movn	$4, $5, $2
	movn	$7, $6, $17
	movn	$2, $0, $4
	movz	$2, $3, $4
	movz	$4, $5, $2
	movz	$17, $6, $7
	movz	$2, $0, $4

	movtn	$2, $3
	movtn	$4, $5
	movtn	$7, $6
	movtn	$2, $0
	movtz	$2, $3
	movtz	$4, $5
	movtz	$17, $6
	movtz	$2, $0

	ehb
	pause

	sync
	sync	1
	sync	4
	sync	13
	sync	31
	sync_wmb
	sync_mb
	sync_acquire
	sync_release
	sync_rmb

	rdhwr	$2, $1
	rdhwr	$3, $5
	rdhwr	$4, $29
	rdhwr	$5, $31

	di
	di	$0
	di	$2
	ei
	ei	$0
	ei	$2

	mfc0	$3, $5
	mfc0	$5, $9, 0
	mfc0	$7, $13, 3
	mfc0	$17, $15, 1
	mfc0	$2, $17, 7
	mfc0	$6, $21
	mtc0	$3, $5
	mtc0	$5, $9, 0
	mtc0	$7, $13, 3
	mtc0	$17, $15, 1
	mtc0	$2, $17, 7
	mtc0	$6, $21

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
