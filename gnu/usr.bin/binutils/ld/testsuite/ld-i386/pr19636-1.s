	.text
	.weak func1
	.weak func2
	.weak func3
	.globl _start
_start:
	cmp func1@GOT(%eax), %eax
	jmp *func2@GOT(%eax)
	call func3@PLT
