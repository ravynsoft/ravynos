# Macros that are disabled without double-precision fp insns.
	
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

d:
	.word 0
	.word 0
