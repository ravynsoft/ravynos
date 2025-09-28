	.abicalls
	.set	nomips16
	.global	foo
	.ent	foo
foo:
	.set noreorder
	.cpload	$25
	.set reorder
	lw	$25,%call16(foo)($28)
	lw	$2,%got(data)($28)
	addiu	$2,$2,%lo(data)
	jrc	$25
	.end	foo

	.global	bar
	.ent	bar
bar:
	.set noreorder
	.cpload	$25
	.set reorder
	lw	$25,%call16(ext)($28)
	jrc	$25
	.end	bar

	.data
data:	.word	0x12345678
