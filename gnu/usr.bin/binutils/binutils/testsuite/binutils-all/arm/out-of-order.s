	.text
	.arm
	.global v1
	.section .func1,"ax",%progbits
	.type v1 %function
	.size v1, 4
v1:
	add r0, r0, r1
	.word 0

	.section .func2,"ax",%progbits
	add r0, r0, r1

	.section .func3,"ax",%progbits
	add r0, r0, r1
	add r0, r0, r1
	add r0, r0, r1
	add r0, r0, r1
	add r0, r0, r1
	.word 0

	.data
	.section .global,"aw",%progbits
	.word 0
	.word 0
	.word 0

	.section .rodata
	.word 0
