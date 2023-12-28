	.abicalls
	.option	pic0
	.global	__start
	.ent	__start
__start:
	jal	foo
	j	ext
	.end	__start

	.global	ext
	.ent	ext
ext:
	lui	$gp,%hi(__gnu_local_gp)
	addiu	$gp,$gp,%lo(__gnu_local_gp)
	lw	$2,%got(local)($gp)
	addiu	$2,$2,%lo(local)
	lw	$25,%call16(bar)($gp)
	jr	$25
	.end	ext

	.data
local:	.word	1
