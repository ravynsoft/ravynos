foo:
	move $r0, $r0
	move $sp, $sp
	movi $r0, -16
	movi $sp, 15
	add $r0, $r0, $r0
	add $r19, $sp, $r19
	sub $r0, $r0, $r0
	sub $r19, $r19, $sp
	addi $r0, $r0, 0
	addi $r19, $r19, 31
	srai $r0, $r0, 0
	srai $r19, $r19, 31
	srli $r0, $r0, 0
	srli $r19, $r19, 31
	slli $r0, $r0, 0
	slli $r7, $r7, 7
	zeb $r0, $r0
	zeb $r7, $r7
	zeh $r0, $r0
	zeh $r7, $r7
	seb $r0, $r0
	seb $r7, $r7
	seh $r0, $r0
	seh $r7, $r7
	andi $r0, $r0, 1
	andi $r7, $r7, 0x7ff
	add $r0, $r0, $r0
	add $r7, $r7, $r7
	sub $r0, $r0, $r0
	sub $r7, $r7, $r7
	addi $r0, $r0, 0
	addi $r7, $r7, 7
	lwi $r0, [$r0 + 0]
	lwi $r7, [$r7 + 28]
	lwi.bi $r0, [$r0], 0
	lwi.bi $r7, [$r7], 28
	lhi $r0, [$r0 + 0]
	lhi $r7, [$r7 + 14]
	lbi $r0, [$r0 + 0]
	lbi $r7, [$r7 + 7]
	swi $r0, [$r0 + 0]
	swi $r7, [$r7 + 28]
	swi.bi $r0, [$r0], 0
	swi.bi $r7, [$r7], 28
	shi $r0, [$r0 + 0]
	shi $r7, [$r7 + 14]
	sbi $r0, [$r0 + 0]
	sbi $r7, [$r7 + 7]
	lwi $r0, [$r0 + 0]
	lwi $r19, [$sp + 0]
	swi $r0, [$r0 + 0]
	swi $r19, [$sp + 0]
	lwi $r0, [$fp + 0]
	lwi $r7, [$fp + 508]
	swi $r0, [$fp + 0]
	swi $r7, [$fp + 508]
	jr $r0
	jr $sp
	ret $r0
	ret $sp
	jral $r0
	jral $sp
	slts $r15, $r0, $r0
	slts $r15, $r19, $sp
	slt $r15, $r0, $r0
	slt $r15, $r19, $sp
	sltsi $r15, $r0, 0
	sltsi $r15, $r19, 31
	slti $r15, $r0, 0
