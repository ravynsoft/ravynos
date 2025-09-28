	.text
	data16 jmp foo
	data16 rex.w jmp foo

bar:
	mov %eax, %ebx

	data16 call foo
	data16 rex.w call foo

	data16 xbegin foo
	data16 rex.w xbegin foo

	lcallq *(%rax)
	ljmpq *(%rcx)
