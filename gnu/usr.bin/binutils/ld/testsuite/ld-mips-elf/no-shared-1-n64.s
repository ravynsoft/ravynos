	.abicalls
	.text
	.globl	__start
	.ent	__start
__start:
	lui	$2,%hi(__gnu_local_gp)
	daddiu	$2,$2,%lo(__gnu_local_gp)
	ld	$2,%got(__gnu_local_gp)($gp)
	ld	$2,%call16(__gnu_local_gp)($gp)
	.end	__start

	.data
	.8byte	__gnu_local_gp
