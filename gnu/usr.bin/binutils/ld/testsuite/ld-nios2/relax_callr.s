# relaxation test for callr

.globl text1
.section text1, "ax", @progbits

	call func
	call func1
	
.section text2, "ax", @progbits
func:
	nop
	br func1
	nop
	nop
	nop
func1:
	nop
