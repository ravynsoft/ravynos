# Check -madd-bnd-prefix option
	.text

	call	foo
	call	*(%eax)
	je	foo
	jmp	foo
	jmp	*(%ebx)
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
	add %eax, %ebx
	loop foo
