	.text
	.arch armv7-m
	.syntax unified
	.global	_start
	.thumb_func
	.type	_start, %function
_start:
	.thumb_func
	.type	thumb1, %function
thumb1:
	movs r0, #:upper8_15:#thumb3
	movs r1, #:upper0_7:#thumb3
	movs r2, #:lower8_15:#thumb1
	movs r3, #:lower0_7:#thumb1
	movs r4, #:lower0_7:#thumb3
	movs r5, #:lower8_15:#thumb3
	movs r6, #:upper0_7:#thumb1
	movs r7, #:upper8_15:#thumb1
	.thumb_func
	.type	thumb2, %function
thumb2:
	movs r0, #:upper8_15:#(thumb3 + 0)
	movs r1, #:upper0_7:#(thumb2 + 1)
	movs r2, #:lower8_15:#(var1 + 255)
	movs r3, #:lower0_7:#var1
	movs r7, #:upper8_15:#var1 + 4
	movs r6, #:upper0_7:#var2
	movs r5, #:lower8_15:#var2 + 0xff
	movs r4, #:lower0_7:#var2 - (-1)
var1:
	.byte 1
var2:
	.byte 2

	.section .far, "ax", %progbits
	.thumb_func
	.type	thumb3, %function
thumb3:
	movs r0, #:upper8_15:#thumb1
	movs r1, #:upper0_7:#thumb2
	movs r2, #:lower8_15:#thumb3
	movs r3, #:lower0_7:#thumb1

