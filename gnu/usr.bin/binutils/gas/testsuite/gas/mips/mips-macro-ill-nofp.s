# Macros that are disabled without floating point.
	
	.text
double_float:
	ldc1	$f2, d
	ldc1	$22, d
	l.d	$f2, d
	li.d	$f2, 1.2 
	li.d	$22, 1.2

	sdc1	$f2, d
	sdc1	$22, d
	s.d	$f2, d

	trunc.w.d $f4,$f6,$4

single_float:
	lwc1	$f2, d
	lwc1	$22, d
	l.s	$f2, d
	li.s	$f2, 1.2 
	li.s	$22, 1.2

	sdc1	$f2, d
	sdc1	$22, d
	s.d	$f2, d

	trunc.w.s $f4,$f6,$4

d:
	.word 0
	.word 0
