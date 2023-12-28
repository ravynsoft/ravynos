	.text
_start:
.L0:
	.nops 62
	.byte 0xf2
	jnc	.L0
	ret
