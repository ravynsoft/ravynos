	.text
	.global start	/* Used by SH targets.  */
start:
	.global _start
_start:
	.global __start
__start:
	.global main	/* Used by HPPA targets.  */
main:
	.globl	_main	/* Used by LynxOS targets.  */
_main:
	.word 0

	.section .text.1, "ax"
	.global bar
bar:
	.word 0
