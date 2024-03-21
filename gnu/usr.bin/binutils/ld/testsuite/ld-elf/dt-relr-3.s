	.text
	.p2align 3
	.globl _start
_start:
	.byte 0

# Section alignment is not a multiple of 2.  Don't use DT_RELR.
	.data
	.p2align 0
	.globl data
data:
	.dc.a data
