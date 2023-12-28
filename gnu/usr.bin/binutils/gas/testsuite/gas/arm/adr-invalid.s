	.text
start:
	adr     r0, var
	adr     r0, undefinedvar
	adrl	r1, var
	adrl	r1, undefinedvar

	.data
	.globl  var
var:
	.word   0x00000000

