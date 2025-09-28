	.org 0
	.section .text
func:
	braid	label
	nop
label:
	.size	func, . - func
