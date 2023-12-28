	.abicalls
	.text
	.globl	__start
	.ent	__start
__start:
	lui	$2,%hi(__gnu_local_gp)
	addiu	$2,$2,%lo(__gnu_local_gp)
	lw	$2,%got(__gnu_local_gp)($gp)
	lw	$2,%call16(__gnu_local_gp)($gp)
	.end	__start

	.data
	.4byte	__gnu_local_gp
