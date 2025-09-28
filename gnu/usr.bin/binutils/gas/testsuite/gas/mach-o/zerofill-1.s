
	.zerofill __DATA, __zf_1

	.globl a
a:	.space 1

	.zerofill __DATA, __zf_2, zfs, 2

	.globl b
b:	.space 1

	.zerofill __DATA, __zf_3, withalign, 2, 3

	.globl c
c:	.space 1

	.zerofill __DATA, __zf_3, withalign1, 4, 3

	.globl d
d:	.space 1
