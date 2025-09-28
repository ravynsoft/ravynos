	.text
foo:
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2
	
	.set softfloat
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2
	.set push

	.set hardfloat
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2

	.set pop
	add.s	$f2,$f2,$f2
	add.d	$f2,$f2,$f2
