	.abicalls
	.symver	foo2,foo@@V2
	.global	foo2
	.ent	foo2
foo2:	jr	$31
	.end	foo2
