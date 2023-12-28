	.file	"a.c"
	.comm	_h, 16
	.comm	_i, 16
	.comm	_j, 16
	.comm	_k, 16
	.section .drectve
	.ascii " -aligncomm:_h,5"
	.ascii " -aligncomm:_i,4"
	.ascii " -aligncomm:_j,3"
	.ascii " -aligncomm:_k,2"

