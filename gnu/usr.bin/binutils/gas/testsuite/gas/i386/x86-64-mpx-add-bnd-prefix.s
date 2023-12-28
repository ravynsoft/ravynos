# Check -madd-bnd-prefix option
	.text

	call	foo
	call	*(%rax)
	je	foo
	jmp	foo
	jmp	*(%rbx)
	ret
foo:
	# Use of REP/REPE prefix - converted to BND with warning
	rep ret
	repe ret
	# Use of REPNE prefix - we shouldn't get any error
	repne ret
	# BND prefix already exists - we shouldn't get any error here
	bnd ret
	bnd call	foo
	# Following instructions can't have BND prefix even if
	# -madd-bnd-prefix is specified
	add %rax, %rbx
	loop foo
