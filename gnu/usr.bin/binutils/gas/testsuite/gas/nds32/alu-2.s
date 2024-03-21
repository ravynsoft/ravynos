foo:
	madd64 $d0, $r0, $r1
	madds64 $d0, $r0, $r1
	mfusr $r0, $pc
	msub64 $d0, $r0, $r1
	msubs64 $d0, $r0, $r1
	mtusr $r0, $pc
	mul $r0, $r1, $r2
	mult32 $d0, $r1, $r2
	mult64 $d0, $r1, $r2
	mults64 $d0, $r1, $r2
	abs $r0, $r1
	ave $r0, $r1, $r2
	bclr $r0, $r1, 1
	bset $r0, $r1, 1
	btgl $r0, $r1, 1
	btst $r0, $r1, 1
	clip $r0, $r1, 1
	clips $r0, $r1, 1
	clo $r0, $r1
	clz $r0, $r1
	max $r0, $r1, $r2
	min $r0, $r1, $r2
	bse $r0, $r1, $r2
	bsp $r0, $r1, $r2
	ffb $r0, $r1, $r2
	ffbi $r0, $r1, 1
	ffmism $r0, $r1, $r2
	flmism $r0, $r1, $r2
	maddr32 $r0, $r0, $r1
	msubr32 $r0, $r1, $r2
	mulr64 $r0, $r1, $r2
	mulsr64 $r0, $r1, $r2
