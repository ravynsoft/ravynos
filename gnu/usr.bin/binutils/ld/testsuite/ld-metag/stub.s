
	.text
	.global __start
__start:
	CALLR D1RtP,_far

	.section .text.pad,"ax"
	.space 0x200000

	.section .text.far,"ax"
	.global _far
_far:
	NOP
