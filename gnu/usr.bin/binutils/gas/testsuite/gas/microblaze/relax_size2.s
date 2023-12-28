	.org 0
	.section .text
func:
	nop
label:
	.size   func, . - func
func2:
	braid	label2
	nop
label2:
	.size	func2, . - func2
