	.data
	.hidden foo
	.globl	foo
foo:
	.byte 20
local:
	.byte 20
	.text
	.globl	bar
	.type	bar, @function
bar:
	lea	foo(%rip), %rax
	lea	local(%rip), %rax
	.size	bar, .-bar
