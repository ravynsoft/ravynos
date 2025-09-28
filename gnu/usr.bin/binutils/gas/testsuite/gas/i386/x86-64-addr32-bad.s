	.text
addr32:
	lea	0x88888888(%rax), %rax
	lea	0x88888888(%rax), %eax
	lea	0x88888888(%eax), %rax

	lea	value(%rax), %rax
	lea	value(%rax), %eax
	lea	value(%eax), %rax

	mov	$value, %rax
	mov	$value, %eax

	.equ	value, 0x99999999
	.end
