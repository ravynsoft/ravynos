	.text
	.globl _start
_start:
.L1:	auipc	a0, %pcrel_hi(data_a)
.L2:	auipc	a1, %pcrel_hi(data_b)
	addi	a0, a0, %pcrel_lo(.L1)
	addi	a1, a1, %pcrel_lo(.L2)

	.data
	.word 0x0
	.globl data_a
data_a:
	.word 0x1

	.section .rodata
	.globl data_b
data_b:
	.word 0x2
