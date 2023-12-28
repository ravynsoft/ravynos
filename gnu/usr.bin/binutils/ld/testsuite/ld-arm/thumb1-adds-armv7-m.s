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
	adds r0, #:upper8_15:#thumb3
	adds r1, #:upper0_7:#thumb3
	adds r2, #:lower8_15:#thumb1
	adds r3, #:lower0_7:#thumb1
	adds r4, #:lower0_7:#thumb3
	adds r5, #:lower8_15:#thumb3
	adds r6, #:upper0_7:#thumb1
	adds r7, #:upper8_15:#thumb1
	.thumb_func
	.type	thumb2, %function
thumb2:
	adds r0, #:upper8_15:#thumb3
	adds r1, #:upper0_7:#(var2 + 1)
	adds r2, #:lower8_15:#(thumb3 + 255)
	adds r3, #:lower0_7:#(var1 + 0xaa)
	adds r7, #:upper8_15:#var1 + 4
	adds r6, #:upper0_7:#thumb3
	adds r5, #:lower8_15:#var2 + 0xff
	adds r4, #:lower0_7:#var2 - (-2)
var1:
	.byte 1
var2:
	.byte 2

	.section .far, "ax", %progbits
	.thumb_func
	.type	thumb3, %function
thumb3:
	adds r0, #:upper8_15:#thumb1
	adds r1, #:upper0_7:#thumb2
	adds r2, #:lower8_15:#thumb3
	adds r3, #:lower0_7:#thumb1

