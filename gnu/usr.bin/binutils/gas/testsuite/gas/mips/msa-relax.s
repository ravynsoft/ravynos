	.text
foo:
	bz.b	$w0, bar
	bz.h	$w1, bar
	bz.w	$w2, bar
	bz.d	$w3, bar
	bnz.b	$w4, bar
	bnz.h	$w5, bar
	bnz.w	$w6, bar
	bnz.d	$w7, bar
	bz.v	$w8, bar
	bnz.v	$w9, bar

	.space	0x20000		# to make a 128kb loop body
bar:
	bz.b	$w10, foo
	bz.h	$w11, foo
	bz.w	$w12, foo
	bz.d	$w13, foo
	bnz.b	$w14, foo
	bnz.h	$w15, foo
	bnz.w	$w16, foo
	bnz.d	$w17, foo
	bz.v	$w18, foo
	bnz.v	$w19, foo

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
