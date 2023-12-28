	.section .init.text,"ax",@progbits
	.global foo
foo:
	call	printk
	lea	printk(%rip), %rax
	.text
printk:
	ret
