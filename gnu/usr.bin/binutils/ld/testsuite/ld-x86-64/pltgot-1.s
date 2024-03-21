	.text
	.globl	plt	
	.type	plt, @function
plt:
	call   *puts@GOTPCREL(%rip)
	jmp	puts@PLT
