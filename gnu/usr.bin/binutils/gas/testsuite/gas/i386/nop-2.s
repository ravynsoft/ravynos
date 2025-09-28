       .text
       .code16
single:
	.nops 0
	nop

pseudo_1:
	.nops 1

pseudo_8:
	.nops 8

pseudo_8_4:
	.nops 8, 4

pseudo_20:
	.nops 20

pseudo_30:
	.nops 30

pseudo_129:
	.nops 129

end:
	xor %eax, %eax
