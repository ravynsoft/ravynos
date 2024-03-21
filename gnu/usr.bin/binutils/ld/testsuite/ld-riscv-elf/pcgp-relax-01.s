	.text
	.globl _start
_start:
	addi	a0, a0, %pcrel_lo(.L2)

	call	_start
.L1:
	auipc	a1, %pcrel_hi(data_g)
	addi	a1, a1, %pcrel_lo(.L1)

	lui	a2, %hi(data_g)
	addi	a2, a2, %lo(data_g)

	lui	a3, %tprel_hi(data_t)
	add	a3, a3, tp, %tprel_add(data_t)
	addi	a3, a3, %tprel_lo(data_t)

.L2:
	auipc   a0, %pcrel_hi(data_g)

	.data
	.word 0x0
	.globl data_g
data_g:
	.word 0x1

	.section .tbss
data_t:
	.word 0x0
