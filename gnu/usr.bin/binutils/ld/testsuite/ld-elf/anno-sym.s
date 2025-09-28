	.text

	.hidden .annobin_hello.c
	.type .annobin_hello.c, STT_NOTYPE
	.equiv .annobin_hello.c, .
	.size .annobin_hello.c, 0
	
	.global _start
_start:
	.nop
	.align 4
	.dc.a foo

