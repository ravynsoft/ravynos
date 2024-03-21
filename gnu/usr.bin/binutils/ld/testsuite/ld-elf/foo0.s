	.text
	.global start	/* Used by SH targets.  */
start:
	.global _start
_start:
	.global __start
__start:
	.global main	/* Used by HPPA targets.  */
main:
	.dc.a 0

	.section .foo,"ax",%progbits
	.p2align 2
	.global foo
	.type	foo,%function
foo:
