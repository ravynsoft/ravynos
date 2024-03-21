	.text
	.globl	plt	
	.type	plt, @function
plt:
	call   *puts@GOT(%ebx)
	jmp	puts@PLT
