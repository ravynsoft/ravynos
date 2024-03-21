	.globl	lib1
	.csect	lib1[RW]
lib1:
	.long	0x11110001

	.globl	_lib2
	.csect	_lib2[RW]
_lib2:
	.long	0x11110002

	.globl	lib3
	.csect	lib3[RW]
lib3:
	.long	0x11110003
