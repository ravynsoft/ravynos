	.text
	.align  2
	.global main
	.type   main, %function
main:
	sub     sp, sp, #16
	mov     x7, 13
	str     w7, [sp,12]
	b       farbranch
	ret
	.size   main, .-main

	.section .e843419, "xa"
	.align  2
	.global farbranch
	.type   farbranch, %function
farbranch:
	sub     sp, sp, #16
	mov     x7, 13
	str     w7, [sp,12]
	b       e843419
	 .fill 4072,1,0
e843419:
	adrp x0, :gottprel:l_tlsievar
	str x7, [x0,12]
	mov	x8, 9
	str x8, [x0, :gottprel_lo12:l_tlsievar]

	add x0, x1, x5
	ldr     w7, [sp,12]
	add     w0, w7, w7
	add     sp, sp, 16
	ret
	.size   farbranch, .-farbranch

# ---

	.section	.tbss,"awT",%nobits
	.align  2
	.type   l_tlsievar, %object
	.size   l_tlsievar, 4
l_tlsievar:
	.zero   4
