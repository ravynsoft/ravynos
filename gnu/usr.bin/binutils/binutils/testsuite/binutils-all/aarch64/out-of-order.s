	.text
	.global v1
	.section .func1,"ax",@progbits
	.type v1 %function
	.size v1, 4
v1:
	add x0, x0, x1
	.word 0

	.section .func2,"ax",@progbits
	add x0, x0, x1

	.section .func3,"ax",@progbits
	add x0, x0, x1
	add x0, x0, x1
	add x0, x0, x1
	add x0, x0, x1
	add x0, x0, x1
	.word 0

	.data
	.section .global,"aw",@progbits
	.xword 0
	.xword 0
	.xword 0

	.section .rodata
	.word 0
