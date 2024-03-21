	.text
	drps

	//
	// HINTS
	//

	nop
	yield
	wfe
	wfi
	sev
	sevl
	clearbhb

	.macro	all_hints from=0, to=127
	hint \from
	.if	\to-\from
	all_hints "(\from+1)", \to
	.endif
	.endm

	all_hints from=0, to=63
	all_hints from=64, to=127

	//
	// SYSL
	//

	sysl	x7, #3, C15, C7, #7

	//
	// BARRIERS
	//

	.macro	all_barriers op, from=0, to=15
	\op	\from
	.if	\to-\from
	all_barriers \op, "(\from+1)", \to
	.endif
	.endm

	all_barriers	op=dsb, from=0, to=15
	all_barriers	op=dmb, from=0, to=15
	all_barriers	op=isb, from=0, to=15

	isb
	isb sy
	ssbb
	pssbb

	dsb oshld
	dsb oshst
	dsb osh
	dsb nshld
	dsb nshst
	dsb nsh
	dsb #0x08
	dsb ishld
	dsb ishst
	dsb ish
	dsb #0x0c
	dsb ld
	dsb st
	dsb sy

	//
	// PREFETCHS
	//

	.macro	all_prefetchs op, from=0, to=31
	\op	\from, LABEL1
	.if	\from < 24
	\op	\from, [sp, x15, lsl #0]
	\op	\from, [x7, w30, uxtw #3]
	.endif
	\op	\from, [x3, #24]
	.if	\to-\from
	all_prefetchs \op, "(\from+1)", \to
	.endif
	.endm

	all_prefetchs	op=prfm, from=0, to=31

	//
	// PREFETCHS with named operation
	//

	.irp op, pld, pli, pst
	.irp l, l1, l2, l3
	.irp t, keep, strm
	prfm	\op\l\t, [x3, #24]
	.endr
	.endr
	.endr

	.inst	0xf8a04817
	.inst	0xf8a04818
