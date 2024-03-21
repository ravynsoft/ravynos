	.text	
	.set	reorder
test:
	fsune.d	$w0,$w1,$w2
	bz.b	$w0, test
	fsune.d	$w0,$w1,$w2
	bz.b	$w1, test
	fsune.d	$w0,$w1,$w2
	bz.b	$w2, test
	add.s	$f0,$f1,$f2
	bz.b	$w0, test
	add.s	$f0,$f1,$f2
	bz.b	$w1, test
	add.s	$f0,$f1,$f2
	bz.b	$w2, test
	add.d	$f0,$f2,$f4
	bz.b	$w0, test
	add.d	$f0,$f2,$f4
	bz.b	$w1, test
	add.d	$f0,$f2,$f4
	bz.b	$w2, test

	fsune.d	$w0,$w1,$w2
	bz.h	$w0, test
	fsune.d	$w0,$w1,$w2
	bz.h	$w1, test
	fsune.d	$w0,$w1,$w2
	bz.h	$w2, test
	add.s	$f0,$f1,$f2
	bz.h	$w0, test
	add.s	$f0,$f1,$f2
	bz.h	$w1, test
	add.s	$f0,$f1,$f2
	bz.h	$w2, test
	add.d	$f0,$f2,$f4
	bz.h	$w0, test
	add.d	$f0,$f2,$f4
	bz.h	$w1, test
	add.d	$f0,$f2,$f4
	bz.h	$w2, test

	fsune.d	$w0,$w1,$w2
	bz.w	$w0, test
	fsune.d	$w0,$w1,$w2
	bz.w	$w1, test
	fsune.d	$w0,$w1,$w2
	bz.w	$w2, test
	add.s	$f0,$f1,$f2
	bz.w	$w0, test
	add.s	$f0,$f1,$f2
	bz.w	$w1, test
	add.s	$f0,$f1,$f2
	bz.w	$w2, test
	add.d	$f0,$f2,$f4
	bz.w	$w0, test
	add.d	$f0,$f2,$f4
	bz.w	$w1, test
	add.d	$f0,$f2,$f4
	bz.w	$w2, test

	fsune.d	$w0,$w1,$w2
	bz.d	$w0, test
	fsune.d	$w0,$w1,$w2
	bz.d	$w1, test
	fsune.d	$w0,$w1,$w2
	bz.d	$w2, test
	add.s	$f0,$f1,$f2
	bz.d	$w0, test
	add.s	$f0,$f1,$f2
	bz.d	$w1, test
	add.s	$f0,$f1,$f2
	bz.d	$w2, test
	add.d	$f0,$f2,$f4
	bz.d	$w0, test
	add.d	$f0,$f2,$f4
	bz.d	$w1, test
	add.d	$f0,$f2,$f4
	bz.d	$w2, test

	fsune.d	$w0,$w1,$w2
	bz.v	$w0, test
	fsune.d	$w0,$w1,$w2
	bz.v	$w1, test
	fsune.d	$w0,$w1,$w2
	bz.v	$w2, test
	add.s	$f0,$f1,$f2
	bz.v	$w0, test
	add.s	$f0,$f1,$f2
	bz.v	$w1, test
	add.s	$f0,$f1,$f2
	bz.v	$w2, test
	add.d	$f0,$f2,$f4
	bz.v	$w0, test
	add.d	$f0,$f2,$f4
	bz.v	$w1, test
	add.d	$f0,$f2,$f4
	bz.v	$w2, test

	fsune.d	$w0,$w1,$w2
	bnz.b	$w0, test
	fsune.d	$w0,$w1,$w2
	bnz.b	$w1, test
	fsune.d	$w0,$w1,$w2
	bnz.b	$w2, test
	add.s	$f0,$f1,$f2
	bnz.b	$w0, test
	add.s	$f0,$f1,$f2
	bnz.b	$w1, test
	add.s	$f0,$f1,$f2
	bnz.b	$w2, test
	add.d	$f0,$f2,$f4
	bnz.b	$w0, test
	add.d	$f0,$f2,$f4
	bnz.b	$w1, test
	add.d	$f0,$f2,$f4
	bnz.b	$w2, test

	fsune.d	$w0,$w1,$w2
	bnz.h	$w0, test
	fsune.d	$w0,$w1,$w2
	bnz.h	$w1, test
	fsune.d	$w0,$w1,$w2
	bnz.h	$w2, test
	add.s	$f0,$f1,$f2
	bnz.h	$w0, test
	add.s	$f0,$f1,$f2
	bnz.h	$w1, test
	add.s	$f0,$f1,$f2
	bnz.h	$w2, test
	add.d	$f0,$f2,$f4
	bnz.h	$w0, test
	add.d	$f0,$f2,$f4
	bnz.h	$w1, test
	add.d	$f0,$f2,$f4
	bnz.h	$w2, test

	fsune.d	$w0,$w1,$w2
	bnz.w	$w0, test
	fsune.d	$w0,$w1,$w2
	bnz.w	$w1, test
	fsune.d	$w0,$w1,$w2
	bnz.w	$w2, test
	add.s	$f0,$f1,$f2
	bnz.w	$w0, test
	add.s	$f0,$f1,$f2
	bnz.w	$w1, test
	add.s	$f0,$f1,$f2
	bnz.w	$w2, test
	add.d	$f0,$f2,$f4
	bnz.w	$w0, test
	add.d	$f0,$f2,$f4
	bnz.w	$w1, test
	add.d	$f0,$f2,$f4
	bnz.w	$w2, test

	fsune.d	$w0,$w1,$w2
	bnz.d	$w0, test
	fsune.d	$w0,$w1,$w2
	bnz.d	$w1, test
	fsune.d	$w0,$w1,$w2
	bnz.d	$w2, test
	add.s	$f0,$f1,$f2
	bnz.d	$w0, test
	add.s	$f0,$f1,$f2
	bnz.d	$w1, test
	add.s	$f0,$f1,$f2
	bnz.d	$w2, test
	add.d	$f0,$f2,$f4
	bnz.d	$w0, test
	add.d	$f0,$f2,$f4
	bnz.d	$w1, test
	add.d	$f0,$f2,$f4
	bnz.d	$w2, test

	fsune.d	$w0,$w1,$w2
	bnz.v	$w0, test
	fsune.d	$w0,$w1,$w2
	bnz.v	$w1, test
	fsune.d	$w0,$w1,$w2
	bnz.v	$w2, test
	add.s	$f0,$f1,$f2
	bnz.v	$w0, test
	add.s	$f0,$f1,$f2
	bnz.v	$w1, test
	add.s	$f0,$f1,$f2
	bnz.v	$w2, test
	add.d	$f0,$f2,$f4
	bnz.v	$w0, test
	add.d	$f0,$f2,$f4
	bnz.v	$w1, test
	add.d	$f0,$f2,$f4
	bnz.v	$w2, test

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
