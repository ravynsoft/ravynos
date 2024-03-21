# Source file used to test the call and callr instructions
.text
.set norelax
.set noat
foo:
	call	func1
	callr	r10
# use external symbol
	.global external
	call	external
func1:
	


