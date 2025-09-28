	.abicalls
	.option	pic0
	.global	__start
	.ent	__start
__start:
	lui	$2,%hi(obj1)
	addiu	$2,$2,%lo(obj1)
	.end	__start
	.word	obj2

	.data
	.word	obj2
	.word	obj3
