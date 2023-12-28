	.text
	leal	my_tls@TLSDESC(%ebx), %eax
	call	foo@PLT
