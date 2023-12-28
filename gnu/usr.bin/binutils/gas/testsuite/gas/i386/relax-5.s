	.section .init.text,"ax",@progbits
	.global foo
foo:
	call	printk@PLT
	call	printk
	.text
printk:
	ret
