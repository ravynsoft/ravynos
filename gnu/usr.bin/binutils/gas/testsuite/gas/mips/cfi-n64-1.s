	.global	foo
	.type	foo,@function
	.ent	foo
foo:
	.cfi_startproc
	jr	$31
	nop
	.cfi_endproc
	.end	foo
