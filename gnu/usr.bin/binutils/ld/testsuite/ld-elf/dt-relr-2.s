	.text
	.p2align 3
	.globl _start
_start:
	.byte 0

	.data
	.p2align 3
	.globl data
data:

	.dc.a .data
	.dc.a 0
	.dc.a data + 2
	.dc.a __ehdr_start + 4
	.dc.a __ehdr_start + 9

	.byte 0
# Offset is not a multiple of 2.  Don't use DT_RELR.
	.dc.a __ehdr_start + 10
