	.weak __start_xx
	.weak __stop_xx

	.text
	.global foo
foo:
	movq	__start_xx@gotpcrel(%rip), %rax
	movq	__stop_xx@gotpcrel(%rip), %rax

	.section xx,"a",unique,0
	.byte 0

	.section xx,"a",unique,1
	.byte 1
