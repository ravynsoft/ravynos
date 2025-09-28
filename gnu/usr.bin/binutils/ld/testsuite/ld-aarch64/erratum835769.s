        .text
        .align  2
        .global main
        .type   main, %function
main:
        stp     x29, x30, [sp, -32]!
        add     x29, sp, 0
        mov     x0, -26
        str     x0, [x29,16]
        mov     x0, 26
        str     x0, [x29,24]
        add     x4, x29, 16
        mov     x0, -1
        mov     x1, 2
        mov     x2, -3
        mov     x3, 4
        bl      a1ldr
        add     x4, x29, 16
        mov     x0, -1
        mov     x1, 2
        mov     x2, -3
        mov     x3, 4
        bl      a5ldr
        mov     w0, 0
        ldp     x29, x30, [sp], 32
        ret
        .size   main, .-main

	.align 2
	.global a1ldr
	.type a1ldr, %function
a1ldr:
	ldr w7, [x4,8]!
	mul w6, w0, w1
	ldr x4, [x4]
	madd x5, x2, x3, x6
	mov x0, x5
	ret
	.size a1ldr, .-a1ldr

	.align 2
	.global a5ldr
	.type a5ldr, %function
a5ldr:
	ldr w7, [x4,8]!
	mul w6, w0, w1
	ldr x4, [x4]
	umaddl x5, w2, w3, x6
	mov x0, x5
	ret
	.size a5ldr, .-a5ldr

	.align 2
	.global a6ldr
	.type a6ldr, %function
a6ldr:
	ldr w7, [x4,8]!
	mul w6, w0, w1
	ldr x4, [x4]
	madd x5, x4, x3, x6
	mov x0, x5
	ret
	.size a6ldr, .-a6ldr

	.align 2
	.global a6ldr
	.type a6ldr, %function
a7str:
	ldr w7, [x4,8]!
	mul w6, w0, w1
	str x4, [x4]
	madd x5, x4, x3, x6
	mov x0, x5
	ret
	.size a7str, .-a7str
