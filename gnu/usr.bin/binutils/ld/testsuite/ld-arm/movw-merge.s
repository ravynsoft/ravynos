	.arch armv7-a
	.syntax unified
	.text
	.global	_start
	.type	_start, %function
_start:
	movw	r0, #:lower16:.LC0
	movt	r0, #:upper16:.LC0
	.thumb
	.global	tfunc
	.type	tfunc, %function
tfunc:
	movw	r0, #:lower16:.LC0
	movt	r0, #:upper16:.LC0

	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
	.ascii "pad"
.LC0:
	.ascii	"inner: cont \000"
