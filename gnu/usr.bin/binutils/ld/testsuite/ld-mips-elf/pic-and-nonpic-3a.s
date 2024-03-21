	.abicalls
	.set	noreorder
	.set	nomips16
	.global	foo
	.ent	foo
foo:
	.cpload	$25
	lw	$25,%call16(foo)($28)
	lw	$2,%got(data)($28)
	jr	$25
	addiu	$2,$2,%lo(data)
	.end	foo

	.set	mips16
	.global	bar
	.ent	bar
bar:
	li	$2,%hi(_gp_disp)
	addiu	$2,$pc,%lo(_gp_disp)
	sll	$2,16
	addu	$2,$2,$3
	lw	$3,%call16(ext)($2)
	move	$28,$2
	jr	$3
	move	$25,$3
	.end	bar

	.data
data:	.word	0x12345678
