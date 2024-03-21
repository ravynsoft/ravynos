	.file	1 "loc-swap-2.s"
	.cfi_startproc
	.ent	foo
	.type	foo,@function
foo:
	.loc	1 7
	move	$5,$6
	.loc	1 9
	.loc	1 10
	jr	$4

	.loc	1 13
	move	$4,$7
	.loc	1 14
	.loc	1 15
	bnez	$4,foo

	.loc	1 17
	li	$5,1
	.end	foo
	.cfi_endproc
