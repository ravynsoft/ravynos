# R_AARCH64_GOT_LD_PREL19 must satisfy condition:
#
#   -2^20 <= G(S) - P < 2^20
#

	.comm	src,1,8

	.global	_start

	.text

_start:
	nop
	ldr	x0, :got:src
