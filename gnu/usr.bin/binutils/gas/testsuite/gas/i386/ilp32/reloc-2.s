	.text
_start:
	movabs	$x+0x123456789, %rax
	movabs	x+0x123456789, %eax

	.data
	.quad x+0x123456789
