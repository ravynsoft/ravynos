	.text
	.globl _start
	.align 3
_start:
	nop
	.LA0: auipc	a5,%pcrel_hi(ll)
	lw	a0,%pcrel_lo(.LA0)(a5)
	lw	a0,%pcrel_lo(.LA0+0x1000)(a5)
	ret
	.globl ll
	.data
	.align 3
	.zero 2024
ll:
	.word 0
	.word 0
