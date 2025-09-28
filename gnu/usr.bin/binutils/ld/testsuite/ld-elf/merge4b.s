/* Check that pointing into padding zeros of string sections works. */
	.section	.rodata.str1.1,"aMS",@progbits,1
	.asciz ""
.LC0:
	.asciz	"foobar"
1:
	.asciz ""
	.asciz "whatever"
	.section .data
	.globl addr_of_str
	.type addr_of_str, @object
addr_of_str:
	.dc.a str
	.globl addr_of_str2
	.type addr_of_str2, @object
addr_of_str2:
	.dc.a 1b
	.section	.rodata
	.type	str, @object
	.size	str, 7
str:
	.asciz	"foobar"
	.section	.note.GNU-stack,"",@progbits
