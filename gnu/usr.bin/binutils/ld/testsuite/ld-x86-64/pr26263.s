	.section .init.text,"ax",@progbits
	.global foo
foo:
	call	printk
	call	printk@PLT
	.text
printk:
	ret
