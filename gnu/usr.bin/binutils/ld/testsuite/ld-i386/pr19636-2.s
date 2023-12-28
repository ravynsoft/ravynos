	.text
	.weak func
	.globl _start
_start:
	cmp func@GOT(%eax), %eax
	jmp *func@GOT(%eax)
	call func@PLT
	cmp $func, %eax
	call func
