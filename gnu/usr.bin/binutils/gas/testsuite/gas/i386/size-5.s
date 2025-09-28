	.text
size:
	mov	$size@size, %eax
	mov	$size@size + val, %eax
	mov	$-size@size, %ecx
	mov	$0 - size@size, %ecx
	mov	$0x100 - size@size, %edx
	mov	$val - size@size, %edx

	lea	size@size, %eax
	lea	size@size + val, %eax
	lea	-size@size, %ecx
	lea	0 - size@size, %ecx
	lea	0x100 - size@size, %edx
	lea	val - size@size, %edx

	ret
	.size size, . - size

	.data
	.p2align 2
	.long	size@size
	.long	size@size + val
	.long	-size@size
	.long	0 - size@size
	.long	0x100 - size@size
	.long	val - size@size

	.long	ext@size
	.long	ext@size + val

	.equ val, 0x1000
