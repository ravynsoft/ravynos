	.section	.rodata
	.align	3
	.type	string1, %object
	.size	string1, 8
local_foo:
	.string	"local_foo"

	.section	.data,"aw",%progbits
	.align	3
	.global	a
	.type	a, %object
	.size	a, 24
a:
	.xword	0xcafecafe
	.xword	local_foo + 0xca
	.xword	0xdeaddead
