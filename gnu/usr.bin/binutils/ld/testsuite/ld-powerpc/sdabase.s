	.text
	.globl _start
_start:

	.section .sdata,"aw",@progbits
	.globl my_sdata
my_sdata:
	.dc.a	_SDA_BASE_
	.dc.a	my_sdata
