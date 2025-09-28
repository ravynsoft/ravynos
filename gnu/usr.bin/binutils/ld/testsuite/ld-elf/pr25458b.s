	.data
	.globl __environ
	.type __environ,%object
__environ:
	.dc.a	0
	.size	__environ, .-__environ
	.weak _environ
	.globl _environ
	.set _environ, __environ
	.weak environ
	.set environ, __environ
