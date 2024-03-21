
	.text
	.global __start
__start:
	CALLR D1RtP,_far

	.section .text.pad,"ax"
	.space 0x200000
	.global pad_end
pad_end:
	.section .text.far,"ax"
	.global _far2
_far2:
	NOP
_far:
	CALLR D1RtP,_far2@PLT
	