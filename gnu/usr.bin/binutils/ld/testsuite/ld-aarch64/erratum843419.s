
        .comm   data0,4,4
	.text
        .align  2
        .global main
        .type   main, %function
main:
        sub     sp, sp, #16
        mov     x7, 13
        str     w7, [sp,12]
        b       e843419
        ret
        .size   main, .-main

        .section .e843419, "xa"
        .align  2
        .global e843419
        .type   e843419, %function
e843419:
        sub     sp, sp, #16
        mov     x7, 13
        str     w7, [sp,12]
        b       e843419_1
         .fill 4072,1,0
e843419_1:
	adrp x0, data0
        str x7, [x0,12]
        mov	x8, 9
        str x8, [x0, :lo12:data0]

        add x0, x1, x5
        ldr     w7, [sp,12]
        add     w0, w7, w7
        add     sp, sp, 16
	b	e835769
        ret
        .size   e843419, .-e843419

        .section .e835769, "xa"
	.align 2
	.global e835769
	.type e835769, %function
e835769:
	ldr w7, [x4,8]!
	mul w6, w0, w1
	ldr x4, [x4]
	madd x5, x2, x3, x6
	mov x0, x5
	ret
	.size e835769, .-e835769

# ---



 

# ---

        .data
data0:
        .fill 8,1,255
        .balign 512
        .fill 4,1,255
        # double word access that crosses a 64 bit boundary
data1: 
        .fill 2,1,255

        # word access that crosses a 64 boundary
data2:
        .fill 2,1,255

data5:
        .fill 4,1,255

        # double word access that crosses a 128 boundary
data3:
        .fill 2,1,255

        # word access that crosses a 128 bit boundary 
data4:
        .fill 2,1,255
data6:
        .fill 496,1,255
