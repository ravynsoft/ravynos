
	.text
	.global _start
_start:
	nop

	.data
my_global_var:
	.byte 10
	.byte 11
	.byte 12
	.byte 13

	.section .pru_irq_map,"",@progbits
my_intc_map:
	.byte 0
	.byte 1
	.byte 19
	.byte 1
	.byte 1
