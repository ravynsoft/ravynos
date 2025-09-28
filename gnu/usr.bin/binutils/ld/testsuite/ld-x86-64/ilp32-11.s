	.section	.data.rel.local,"aw",@progbits
	.align 8
.Ljmp:
	.quad	func + 0x7fffffff

	.text
	.space	0x1000
	.type	func, @function
	.global	func
	.hidden	func
func:
	ret
