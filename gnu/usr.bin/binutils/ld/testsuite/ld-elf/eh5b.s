	.text
	.cfi_startproc simple
	.long 0
	.cfi_def_cfa 0, 16
	.long 0
	.cfi_endproc

	.cfi_startproc simple
	.cfi_personality 3, my_personality_v0
	.long 0
	.cfi_def_cfa 0, 16
	.cfi_endproc

	.cfi_startproc simple
	.long 0
	.cfi_def_cfa 0, 16
	.long 0
	.cfi_endproc

	.cfi_startproc simple
	.cfi_personality 3, my_personality_v0
	.cfi_lsda 12, 0xdeadbeef
	.long 0
	.cfi_def_cfa 0, 16
	.cfi_endproc

	.globl main
	.globl _main
	.globl start
	.globl _start
	.globl __start
main:
_main:
start:
_start:
__start:
	.long 0
