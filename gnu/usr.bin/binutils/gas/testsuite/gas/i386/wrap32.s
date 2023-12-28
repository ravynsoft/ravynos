	.text
wrap:
	mov	$500 - 0x100, %eax
	mov	$500 + 0xffffff00, %edx
	mov	$val - 0x100, %eax
	mov	$val + 0xffffff00, %edx
	mov	$sym - 0x100, %eax
	mov	$sym + 0xffffff00, %edx
	mov	$sym + 500 - 0x100, %eax
	mov	$sym + 500 + 0xffffff00, %edx

	movl	$500 - 0x100, (%eax)
	movl	$500 + 0xffffff00, (%edx)
	movl	$val - 0x100, (%eax)
	movl	$val + 0xffffff00, (%edx)
	movl	$sym - 0x100, (%eax)
	movl	$sym + 0xffffff00, (%edx)
	movl	$sym + 500 - 0x100, (%eax)
	movl	$sym + 500 + 0xffffff00, (%edx)

	add	$500 - 0x100, %ecx
	add	$500 + 0xffffff00, %edx
	add	$val - 0x100, %ecx
	add	$val + 0xffffff00, %edx
	add	$sym - 0x100, %ecx
	add	$sym + 0xffffff00, %edx
	add	$sym + 500 - 0x100, %ecx
	add	$sym + 500 + 0xffffff00, %edx

	addl	$500 - 0x100, (%eax)
	addl	$500 + 0xffffff00, (%edx)
	addl	$val - 0x100, (%eax)
	addl	$val + 0xffffff00, (%edx)
	addl	$sym - 0x100, (%eax)
	addl	$sym + 0xffffff00, (%edx)
	addl	$sym + 500 - 0x100, (%eax)
	addl	$sym + 500 + 0xffffff00, (%edx)

	ret

	.data
	.long 500 - 0x100
	.long 500 + 0xffffff00
	.long val - 0x100
	.long val + 0xffffff00
	.long sym - 0x100
	.long sym + 0xffffff00
	.long sym + 500 - 0x100
	.long sym + 500 + 0xffffff00

	.slong 500 - 0x8fffff00
	.slong 500 + 0x7fffff00
	.slong val - 0x8fffff00
	.slong val + 0x7fffff00
	.slong sym - 0x8fffff00
	.slong sym + 0x7fffff00
	.slong sym + 500 - 0x8fffff00
	.slong sym + 500 + 0x7fffff00

	.equ val, 400
