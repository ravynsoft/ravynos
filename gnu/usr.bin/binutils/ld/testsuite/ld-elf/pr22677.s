	.section .preinit_array.01000,"aw",%preinit_array
	.p2align 2
	.word 0

	.section .init_array.01000,"aw",%init_array
	.p2align 2
	.word 0

	.section .fini_array.01000,"aw",%fini_array
	.p2align 2
	.word 0

	.globl main
	.globl start
	.globl _start
	.globl __start
	.globl foo
	.text
main:
start:
_start:
__start:
foo:
	.word 0
