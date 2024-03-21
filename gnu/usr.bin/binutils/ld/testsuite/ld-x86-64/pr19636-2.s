	.text
	.weak func1
	.weak func2
	.weak func3
	.globl _start
_start:
	cmp func1@GOTPCREL(%rip),%rax
	jmp *func2@GOTPCREL(%rip)
	call func3@PLT
