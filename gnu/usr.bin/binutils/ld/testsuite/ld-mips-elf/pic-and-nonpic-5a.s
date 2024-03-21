	.abicalls
	.option	pic0
	.global	__start
	.ent	__start
__start:
	lui	$2,%hi(foo)
	addiu	$2,$2,%lo(foo)
	lui	$2,%hi(obj1)
	addiu	$2,$2,%lo(obj1)
	.end	__start

	.data
	.word	foo
	.word	bar
	.word	obj1
	.word	obj2
