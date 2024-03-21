	.text
foo:
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2

	.set singlefloat
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2
	.set push

	.set doublefloat
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2

	.set pop
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2
