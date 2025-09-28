	.abicalls
	.option	pic0
	.global	obj1
	.global	obj2
	.global	obj3

	.type	obj1,%object
	.type	obj2,%object
	.type	obj3,%object

	.size	obj1,8
	.size	obj2,4
	.size	obj3,16

	.data
obj1:
	.word	1, 2
obj2:
	.word	3
obj3:
	.word	5, 6, 7, 8

