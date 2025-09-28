	.globl bar
bar:
	mov foo@GOTPCREL(%rip), %rax

	.data
xxx:
	.long foo
	.long xxx
