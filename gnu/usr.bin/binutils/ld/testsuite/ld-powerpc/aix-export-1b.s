	.globl	main1
	.csect	main1[RW]
main1:
	.long	lib1

	.globl	_main2
	.csect	_main2[RW]
_main2:
	.long	_lib2
