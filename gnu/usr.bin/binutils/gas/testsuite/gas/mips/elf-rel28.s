# By default test ld/sd.

# If defined, test lld/scd instead.
	.ifdef	tlldscd
	.macro	ld ops:vararg
	lld	\ops
	.endm
	.macro	sd ops:vararg
	scd	\ops
	.endm
	.endif

	.ent	foo
foo:
	# Many of these do not make conceptual sense, but they should
	# at least assemble.
	ld	$4,%call_hi(bar)($4)
	ld	$4,%call_lo(bar)($4)
	ld	$4,%call16(bar)($4)
	ld	$4,%got_disp(bar)($4)
	ld	$4,%got_page(bar)($4)
	ld	$4,%got_ofst(bar)($4)
	ld	$4,%got_hi(bar)($4)
	ld	$4,%got_lo(bar)($4)
	ld	$4,%got(bar)($4)
	ld	$4,%gp_rel(bar)($4)
	ld	$4,%half(bar)($4)
	ld	$4,%highest(bar)($4)
	ld	$4,%higher(bar)($4)
	ld	$4,%neg(bar)($4)
	ld	$4,%tlsgd(bar)($4)
	ld	$4,%tlsldm(bar)($4)
	ld	$4,%dtprel_hi(bar)($4)
	ld	$4,%dtprel_lo(bar)($4)
	ld	$4,%tprel_hi(bar)($4)
	ld	$4,%tprel_lo(bar)($4)
	ld	$4,%gottprel(bar)($4)

	sd	$4,%call_hi(bar)($4)
	sd	$4,%call_lo(bar)($4)
	sd	$4,%call16(bar)($4)
	sd	$4,%got_disp(bar)($4)
	sd	$4,%got_page(bar)($4)
	sd	$4,%got_ofst(bar)($4)
	sd	$4,%got_hi(bar)($4)
	sd	$4,%got_lo(bar)($4)
	sd	$4,%got(bar)($4)
	sd	$4,%gp_rel(bar)($4)
	sd	$4,%half(bar)($4)
	sd	$4,%highest(bar)($4)
	sd	$4,%higher(bar)($4)
	sd	$4,%neg(bar)($4)
	sd	$4,%tlsgd(bar)($4)
	sd	$4,%tlsldm(bar)($4)
	sd	$4,%dtprel_hi(bar)($4)
	sd	$4,%dtprel_lo(bar)($4)
	sd	$4,%tprel_hi(bar)($4)
	sd	$4,%tprel_lo(bar)($4)
	sd	$4,%gottprel(bar)($4)
	.end	foo

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
