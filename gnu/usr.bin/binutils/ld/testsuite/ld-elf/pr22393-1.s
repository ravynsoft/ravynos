	.section .rodata,"a",%progbits
	.globl	fx1
	.type	fx1, %object
fx1:
	.zero	20
	.section .data.rel.ro,"aw",%progbits
	.globl	px1
	.type	px1, %object
px1:
	.dc.a	fx1

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
