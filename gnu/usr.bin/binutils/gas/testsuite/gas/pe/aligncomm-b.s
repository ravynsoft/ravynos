	.file	"a.c"
	.comm	_h, 16, 8
	.comm	_i, 16, 4
	.comm	_j, 16, 2
	.comm	_k, 16, 1
	.section .drectve
	.ascii " -aligncomm:_h,5"
	.ascii " -aligncomm:_i,4"
	.ascii " -aligncomm:_j,3"
	.ascii " -aligncomm:_k,2"

