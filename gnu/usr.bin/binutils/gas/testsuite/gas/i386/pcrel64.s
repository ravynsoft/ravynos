	.text
	.code64
pcrel:
	call	target
	jmp	target
	jz	target
	xbegin	target
	mov	target(%rip), %eax
	mov	target(%eip), %eax
	mov	$target-., %rax
	mov	$target-., %eax

	data16 xbegin target

	call	target+0x7ffff000
	jmp	target+0x7ffff000
	jz	target+0x7ffff000
	xbegin	target+0x7ffff000
	mov	target+0x7ffff000(%rip), %eax
	mov	$target+0x7ffff000-., %rax

	mov	target+0x7ffff000(%eip), %eax
	mov	$target+0x7ffff000-., %eax

	.fill 0x8000, 1, 0xcc
target:
	ret
