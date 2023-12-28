
	.text
	.global __start
__start:
	CALLR D1RtP,_far
	CALLR D1RtP,_lib_func
	CALLR D1RtP,_far2

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

	.data
	.balign 4
	.type _app_data,@object
	.size _app_data,4
_app_data:
	.long _lib_data
