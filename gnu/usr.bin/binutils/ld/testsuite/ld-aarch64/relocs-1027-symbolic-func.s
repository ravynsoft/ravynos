	.text
	.global tempy
	.type   tempy, %function
tempy:
	.size   tempy, .-tempy
	.section	.data.rel
	.align 3
	.type   tempy_ptr, %object
	.size   tempy_ptr, 8
tempy_ptr:
	.xword	tempy
