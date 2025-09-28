	.text
	.globl	bar
	.type	bar, @function
bar:
	leaq	__FUNCTION__.2215(%rip), %rdi
	jmp   *puts@GOTPCREL(%rip)
	.size	bar, .-bar
	.globl	plt
	.type	plt, @function
plt:
	leaq	__FUNCTION__.2219(%rip), %rdi
	subq	$8, %rsp
	call   *puts@GOTPCREL(%rip)
	leaq	__FUNCTION__.2219(%rip), %rdi
	addq	$8, %rsp
	jmp	*puts@GOTPCREL(%rip)
	.size	plt, .-plt
	.section	.rodata
	.type	__FUNCTION__.2219, @object
	.size	__FUNCTION__.2219, 4
__FUNCTION__.2219:
	.string	"plt"
	.type	__FUNCTION__.2215, @object
	.size	__FUNCTION__.2215, 4
__FUNCTION__.2215:
	.string	"bar"
	.section	.note.GNU-stack
